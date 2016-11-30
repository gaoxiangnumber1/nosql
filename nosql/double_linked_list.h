#ifndef NOSQL_SRC_DOUBLE_LINKED_LIST_H_
#define NOSQL_SRC_DOUBLE_LINKED_LIST_H_

#ifndef CAST
#define CAST(type) (type)
#endif

typedef struct ListNode
{
	struct ListNode *previous_;
	struct ListNode *next_;
	void *value_;
} ListNode;

typedef struct ListIterator
{
	ListNode *next_;
	int direction_;
} ListIterator;

#define FROM_HEAD_TO_TAIL 0 // Iterate from head to tail.
#define FROM_TAIL_TO_HEAD 1 // Iterate from tail to head.

typedef struct List
{
	ListNode *head_; // The first node in this linked list.
	ListNode *tail_; // The last node.
	int length_; // The number of nodes in this list.
	void* (*Duplicate) (const void*); // ListDuplicate(List*)
	void* (*Free) (void*); // ListFree(List*); ListDeleteNode(List*, ListNode*)
	void* (*Match) (const void*, const void*); // ListSearchKey(List*, void*)
} List;

// Functions implemented as macros.
#define ListGetDuplicateMethod(list) ((list)->Duplicate)
#define ListSetDuplicateMethod(list, method) ((list)->Duplicate = (method))
#define ListGetFreeMethod(list) ((list)->Free)
#define ListSetFreeMethod(list, method) ((list)->Free = (method))
#define ListGetMatchMethod(list) ((list)->Match)
#define ListSetMatchMethod(list, method) ((list)->Match = (method))
#define ListLength(list) ((list)->length_)
#define ListHeadNode(list) ((list)->head_)
#define ListTailNode(list) ((list)->tail_)
#define ListPreviousNode(node) ((node)->previous_)
#define ListNextNode(node) ((node)->next_)
#define ListNodeValue(node) ((node)->value_)

// Create a new double linked list and return the pointer to it.
List *ListCreate();
// Free the whole list(including all its nodes) memory.
void ListFree(List *list);
// Create a new node whose value_ is value and add it to the head of list.
List *ListAddHeadNode(List *list, const void *value);
// Create a new node whose value_ is value and add it to the tail of list.
List *ListAddTailNode(List *list, const void *value);
// Create a new node whose value_ is value and insert it to the list according to after.
List *ListInsertNode(List *list, ListNode *old_node, const void *value, int after);
// Return a list iterator. ListNextNode(iterator) return the next element of the list.
ListIterator *ListGetIterator(List *list, int direction);
// Return a pointer to the next element of an iterator.
ListNode *ListNextIterateNode(ListIterator *iterator);
// Free the iterator memory.
void ListFreeIterator(ListIterator *iterator);
// Return a pointer to the node that matches the given key.
ListNode *ListSearchKey(List *list, const void *key);
// Return list[index] where 0 is the head, 1 is the element next to head and so on.
ListNode *ListIndex(List *list, int index);
// Remove the specified node from the specified list.
void ListDeleteNode(List *list, ListNode *node);
// Rotate the list removing the tail node and inserting it to the head.
void ListRotate(List *list);
// Return a pointer to a copy of the original list.
List *ListDuplicate(List *origin);

#endif // NOSQL_SRC_DOUBLE_LINKED_LIST_H_
