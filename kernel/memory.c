/*************************************************************************
	> File Name: memory.c
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月11日 星期三 15时00分42秒
 ************************************************************************/

#include "memory.h"
#include "stdint.h"
#include "sync.h"
#include "print.h"

#define PG_SIZE 4096

//0xc009f000是内核主线程栈顶, 0xc009e000是内核主线程的PCB
//一个页框大小的位图可表示128MB内存, 位图位置安排在地址0xc009a000
//这样本系统最大支持4个页框的位图
#define MEM_BITMAP_BASE 0xc009a000

//0xc0000000是内核从虚拟内存3G起
//0x100000意指跨过低端1MB内存, 是虚拟地址在逻辑上连续
#define K_HEAP_START 0xc0100000

#define  PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define  PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

//内存池结构, 生成两个实例用于内核内存池和用户内存池
struct pool {
    struct bitmap pool_bitmap;         //位图结构, 管理物理内存
    uint32_t phy_addr_start;           //本内存池所管理物理内存的起始地址
    uint32_t pool_size;                //本内存池字节容量
    struct lock lock;                  //申请内存时互斥
};

struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;

struct arena {
    struct mem_block_desc *desc;
    uint32_t cnt;
    bool large;
};

struct mem_block_desc k_block_descs[DESC_CNT];
//在pf表示的虚拟内存池中申请pg_cnt个虚拟页
//成功则返回虚拟页的起始地址, 失败则返回NULL
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt)
{
    int vaddr_start = 0, bit_idx_start= -1;

    uint32_t cnt = 0;
    if (pf == PF_KERNEL) {
        //bit_idx_start指起始下标
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;
        }
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    }
    else {        //用户内存池, 实现用户进程
        struct task_struct *cur = running_thread();
        bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;
        }
        while (cnt < pg_cnt) {
            bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        }
        vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    }
    return (void*)vaddr_start;
}

//得到虚拟地址vaddr对应的pde(页目录表)指针
uint32_t* pde_ptr(uint32_t vaddr)
{
    uint32_t* pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

//得到虚拟地址vaddr对应的pte(页表)指针
//指针的值也就是虚拟地址, 返回的能够访问vaddr所在pte的虚拟地址
uint32_t* pte_ptr(uint32_t vaddr)
{
    uint32_t* pte = (uint32_t *)(0xffc00000 +((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return pte;
}

//在m_pool指向的物理内存池中分配一个物理页
//成功则返回页框的物理地址,失败则返回NULL
static void *palloc(struct pool* m_pool) 
{
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);
    if (bit_idx == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
    uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
    return (void *)page_phyaddr;
}

//页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射
static void page_table_add(void* _vaddr, void *_page_phyaddr)
{
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pde = pde_ptr(vaddr);
    uint32_t *pte = pte_ptr(vaddr);

    if (*pde & 0x00000001) {
        ASSERT(!(*pte & 0x00000001));
        if (!(*pte & 0x00000001)) {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
        else {
            PANIC("pte repeat");
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
    }
    else {
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W| PG_P_1);

        memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

//分配pg_cnt个页空间, 成功则返回起始虚拟地址, 失败则返回NULL
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt)
{
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);

    //****************malloc_page的原理是三个动作合成的合成*****************
    //1. 通过vaddr_get在虚拟内存池中申请虚拟地址
    //2. 通过palloc在物理内存池中申请物理页
    //3. 通过page_table_add将以上得到的虚拟地址和物理地址在页表中完成映射
    void *vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    while (cnt-- > 0) {
        void *page_phyaddr = palloc(mem_pool);
        //put_int((uint32_t)page_phyaddr);
        if (page_phyaddr == NULL) {
            return NULL;
        }
        page_table_add((void *)vaddr, page_phyaddr);
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

void *get_kernel_pages(uint32_t pg_cnt)
{
    lock_acquire(&kernel_pool.lock);
    void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pg_cnt * PG_SIZE);
    }
    lock_release(&kernel_pool.lock);
    return vaddr;
}

//在用户空间中申请4K内存, 并返回其虚拟地址
void *get_user_pages(uint32_t pg_cnt)
{
    lock_acquire(&user_pool.lock);
    void *vaddr = malloc_page(PF_USER, pg_cnt);
    memset(vaddr, 0, pg_cnt * PG_SIZE);
    lock_release(&user_pool.lock);
    return vaddr;
}

//申请一页内存, 用vaddr映射到该页, 也就是说我们可以指定虚拟地址
//将地址vaddr与pf池中的物理地址相关联, 仅支持一页分配
void *get_a_page(enum pool_flags pf, uint32_t vaddr)
{
    struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    struct task_struct *cur = running_thread();
    int32_t bit_idx = -1;

    //若当前是用户进程
    if (cur->pgdir != NULL && pf == PF_USER) {
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);
    } 
    else if (cur->pgdir == NULL && pf == PF_KERNEL) {
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
    } 
    else {
        PANIC("get a page: not allow kernel alloc userspace or user alloc kernelspace by get_a_page");   
    }
    void *page_phyaddr = palloc(mem_pool);
    if (page_phyaddr == NULL) {
        return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    lock_release(&mem_pool->lock);
    return (void *)vaddr;
}

//得到虚拟地址映射的物理地址
uint32_t addr_v2p(uint32_t vaddr)
{
    uint32_t *pte = pte_ptr(vaddr);
    //去掉其低12位的页表项属性 + 虚拟地址vaddr的低12位
    return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

static void mem_pool_init(uint32_t all_mem) 
{
    lock_init(&kernel_pool.lock);
    lock_init(&user_pool.lock);
    //用来记录页目录表和页表占用的字节大小
    //第0个和第768个页目录项指向同一个页表, 共享这一页框的空间
    //页目录表中最后一个页目录项指向页目录项本身
    //769~1022个页目录项共254
    //1 + 1 + 254 = 256
    uint32_t page_table_size = PG_SIZE *256;
    //记录当前可以使用的物理内存地址
    uint32_t used_mem = page_table_size + 0x100000;
    //用来存储目前可用的内存字节数
    uint32_t free_mem = all_mem - used_mem;
    //用来保存可用内存字节数free_mem转换成对的物理页数
    uint16_t all_free_pages = free_mem / PG_SIZE;

    //为简化操作, 余数不操作, 坏处是这样做会丢失内存, 好处是不用做内存的越界检查. 
    //因为位图表示的内存少于实际物理内存
    //kernel可用的物理页数
    uint16_t kernel_free_pages = all_free_pages / 2;
    //user可用的物理页数
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    //kernel bitmap的长度, 位图中一位表示一页
    uint32_t kbm_length = kernel_free_pages / 8;
    //user bitmap的长度
    uint32_t ubm_length = user_free_pages / 8;
    //内核可用的物理内存起始地址
    uint32_t kp_start = used_mem;
    //user可用的物理内存起始地址
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

    kernel_pool. phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    user_pool.pool_size = user_free_pages * PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;


    //MEM_BITMAP_BASE定义出解释了0xc009a000的出现的原因
    kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
    //用户内存池的位图紧跟在内核内存池位图之后
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
    kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
}

void block_desc_init(struct mem_block_desc *desc_array)
{
    uint16_t desc_idx, block_size = 16;
    for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
        desc_array[desc_idx].block_size = block_size;
        desc_array[desc_idx].blocks_per_arena = (PG_SIZE - sizeof(struct arena)) / block_size;
        list_init(&desc_array[desc_idx].free_list);
        block_size *= 2;
    }
}

static struct mem_block *arena2block(struct arena *a, uint32_t idx)
{
    return (struct mem_block *)((uint32_t)a + sizeof(struct arena) + idx * a->desc->block_size);
}

static struct arena *block2arena(struct mem_block *b)
{
    return (struct arena *)((uint32_t)b & 0xfffff000);
}

void *sys_malloc(uint32_t size)
{
    enum pool_flags PF;
    struct pool *mem_pool;
    uint32_t pool_size;
    struct mem_block_desc *descs;
    struct task_struct *cur_thread = running_thread();

    if (cur_thread->pgdir == NULL) {
        PF = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        mem_pool = &kernel_pool;
        descs = k_block_descs;
    }
    else {
        PF = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }

    if (!(size > 0 && size < pool_size)) {
        return NULL;
    }
    struct arena *a;
    struct mem_block *b;
    lock_acquire(&mem_pool->lock);

    if (size > 1024) {
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PG_SIZE);

        a = malloc_page(PF, page_cnt);
        if (a != NULL) {
            memset(a, 0, page_cnt * PG_SIZE);
            a->desc = NULL;
            a->cnt = page_cnt;
            a->large = true;
            lock_release(&mem_pool->lock);
            return (void *)(a + 1);
        }
        else {
            lock_release(&mem_pool->lock);
            return NULL;
        }
    }
    else {
        uint8_t desc_idx;
        for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
            if (size <= descs[desc_idx].block_size) {
                break;
            }
        }
        if (list_empty(&descs[desc_idx].free_list)) {
            a = malloc_page(PF, 1);
            if (a == NULL) {
                lock_release(&mem_pool->lock);
                return NULL;
            }
            memset(a, 0, PG_SIZE);
            a->desc = &descs[desc_idx];
            a->large = false;
            a->cnt = descs[desc_idx].blocks_per_arena;
            uint32_t block_idx;

            enum intr_status old_status = intr_disable();

            for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena; block_idx++) {
                b = arena2block(a, block_idx);
                ASSERT(!elem_find(&a->desc->free_list, &b->free_elem));
                list_append(&a->desc->free_list, &b->free_elem);
            }
            intr_set_status(old_status);
        }
        b = elem2entry(struct mem_block, free_elem, list_pop(&(descs[desc_idx].free_list)));
        memset(b, 0, descs[desc_idx].block_size);

        a= block2arena(b);
        a->cnt--;
        lock_release(&mem_pool->lock);
        return (void *)b;
    }
}

//将物理地址pg_phy_addr回收到物理内存池
void pfree(uint32_t pg_phy_addr)
{
    struct pool *mem_pool;
    uint32_t bit_idx = 0;
    if (pg_phy_addr >= user_pool.phy_addr_start) {
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
    }
    else {
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
    }
    bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0);
}

//去掉页表中虚拟地址vaddr的映射, 只去掉vaddr对应的pte
static void page_table_pte_remove(uint32_t vaddr)
{
    uint32_t *pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;
    asm volatile("invlpg %0"::"m"(vaddr):"memory");
}

// 在虚拟地址池中释放以_vaddr起始的连续pg_cnt个虚拟页地址
static void vaddr_remove(enum pool_flags pf, void *_vaddr, uint32_t pg_cnt)
{
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;

    if (pf == PF_KERNEL) {
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
        }
    }
    else {
        struct task_struct *cur_thread = running_thread();
        bit_idx_start = (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PG_SIZE;
        while (cnt < pg_cnt) {
            bitmap_set(&cur_thread->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
        }
    }
}

void mfree_page(enum pool_flags pf, void *_vaddr, uint32_t pg_cnt)
{
    uint32_t pg_phy_addr;
    uint32_t vaddr = (int32_t)_vaddr, page_cnt = 0;
    ASSERT(pg_cnt >= 1 && vaddr % PG_SIZE == 0);
    pg_phy_addr = addr_v2p(vaddr);

    ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= 0x102000);

    if (pg_phy_addr >= user_pool.phy_addr_start) {
        vaddr -= PG_SIZE;
        while (page_cnt < pg_cnt) {
            vaddr += PG_SIZE;
            pg_phy_addr = addr_v2p(vaddr);
            ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= user_pool.phy_addr_start);
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
    else {
        vaddr -= PG_SIZE;
        while (page_cnt < pg_cnt) {
            vaddr += PG_SIZE;
            pg_phy_addr = addr_v2p(vaddr);

            ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= kernel_pool.phy_addr_start
                                                && pg_phy_addr < user_pool.phy_addr_start);
            pfree(pg_phy_addr);
            page_table_pte_remove(vaddr);
            page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
}

void sys_free(void *ptr)
{
    ASSERT(ptr != NULL);
    if (ptr != NULL) {
        enum pool_flags PF;
        struct pool *mem_pool;
        if (running_thread()->pgdir == NULL) {
            ASSERT((uint32_t)ptr >= K_HEAP_START);
            PF = PF_KERNEL;
            mem_pool = &kernel_pool;
        }
        else {
            PF = PF_USER;
            mem_pool = &user_pool;
        }

        lock_acquire(&mem_pool->lock);
        struct mem_block *b = ptr;
        struct arena *a = block2arena(b);

        ASSERT(a->large == 0 || a->large == 1);
        if (a->desc == NULL && a->large == true) {
            mfree_page(PF, a, a->cnt);
        }
        else {
            list_append(&a->desc->free_list, &b->free_elem);

            if (++a->cnt == a->desc->blocks_per_arena) {
                uint32_t block_idx;
                for (block_idx = 0; block_idx < a->desc->blocks_per_arena; block_idx++) {
                    struct mem_block *b = arena2block(a, block_idx);
                    ASSERT(elem_find(&a->desc->free_list, &b->free_elem));
                    list_remove(&b->free_elem);
                }
                mfree_page(PF, a, 1);
            }
        }
        lock_release(&mem_pool->lock);
    }
}

//内存管理部分初始化入口
void mem_init()
{
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
    //put_int(mem_bytes_total);
    mem_pool_init(mem_bytes_total);
    block_desc_init(k_block_descs);
    put_str("mem_init done\n");

}
