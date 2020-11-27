/*************************************************************************
	> File Name: list.h
	> Author:jieni 
	> Mail: 
	> Created Time: 2020年11月13日 星期五 21时46分39秒
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H
#include "stdint.h"
//为了将结点元素转换成实际元素项
#define offset(struct_type, member) (int)(&((struct_type*)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
        (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

/******************定义链表成员结构**************
 * 节点中不需要数据成员, 只要求前驱和后继节点指针*/
struct list_elem {
    struct list_elem* prev;
    struct list_elem* next;
};

//这两个元素不用来表示数据, 仅有标志链表位置功能
//head.next表示第一个元素
//tail.prev表示最后一个元素
struct list {
    struct list_elem head;
    struct list_elem tail;
};

//自定义函数类型function, 由于在list_traversal中用来做回调
typedef int (function)(struct list_elem*, int arg);

void list_init(struct list*);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* plist, struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list *plist, struct list_elem *elem);
void list_remove(struct list_elem *pelem);
struct list_elem *list_pop(struct list *plist);
int list_empty(struct list *plist);
uint32_t list_len(struct list *plist);
struct list_elem *list_traversal(struct list *plist, function func, int arg);
int elem_find(struct list *plist, struct list_elem *obj_elem);
#endif
