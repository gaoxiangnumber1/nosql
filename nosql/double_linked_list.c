#include "double_linked_list.h"

#include <stdio.h>

#include "memory.h"

//#define NULL 0

// Create a new double linked list and return the pointer to it.
// O(1)
List *ListCreate()
{
	return Calloc(sizeof(List)); // Calloc initialize all allocated memory to 0.
}

// Free the whole list(including all its nodes) memory.
// O(N)
void ListFree(List *list)
{
	unsigned long length = list->length_;
	ListNode *current_node = list->head_, *next_node = NULL;
	while(length--)
	{
		next_node = current_node->next_;
		list->Free ? list->Free(current_node->value_) : 0;
		Free(current_node);
		current_node = next_node;
	}
	Free(list);
}

// Create a new node whose value_ is value and add it to the head of list.
//O(1)
List *ListAddHeadNode(List *list, void *value)
{
	ListNode *node;
	if((node = Malloc(sizeof(ListNode))) == NULL)
	{
		return NULL;
	}
	node->value_ = value;
	if(list->length_ == 0) // Empty list.
	{
		node->previous_ = node->next_ = NULL;
		list->head_ = list->tail_ = node;
	}
	else
	{
		node->previous_ = NULL;
		node->next_ = list->head_;
		list->head_->previous_ = node;
		list->head_ = node;
	}
	++list->length_; // Update list's length
	return list;
}

// Create a new node whose value_ is value and add it to the tail of list.
//O(1)
List *ListAddTailNode(List *list, void *value)
{
	ListNode *node;
	if((node = Malloc(sizeof(ListNode))) == NULL)
	{
		return NULL;
	}
	node->value_ = value;
	if(list->length_ == 0) // Empty list.
	{
		node->previous_ = node->next_ = NULL;
		list->head_ = list->tail_ = node;
	}
	else
	{
		node->previous_ = list->tail_;
		node->next_ = NULL;
		list->tail_->next_ = node;
		list->tail_ = node;
	}
	++list->length_; // Update list's length
	return list;
}

// Create a new node whose value_ is value and insert it to the list according to after:
// insert before old_node if after == 0; otherwise, insert after old_node.
//O(1)
List *ListInsertNode(List *list, ListNode *old_node, void *value, int after)
{
	ListNode *new_node;
	if((new_node = Malloc(sizeof(ListNode))) == NULL)
	{
		return NULL;
	}
	new_node->value_ = value;
	// Set new node's previous and next links according to after.
	new_node->previous_ = (after == 0 ? old_node->previous_ : old_node);
	new_node->next_ = (after == 0 ? old_node : old_node->next_);
	// Update old_node's, its neighbor's, and list's(if needed) links.
	if(after == 0) // Insert before old_node
	{
		if(list->head_ == old_node) // old_node is head node, update list.
		{
			list->head_ = new_node;
		}
		else // old_node->previous_ != NULL, update its previous node.
		{
			old_node->previous_->next_ = new_node;
		}
		old_node->previous_ = new_node;
	}
	else // Insert after old_node.
	{
		if(list->tail_ == old_node) // old_node is tail node, update list.
		{
			list->tail_ = new_node;
		}
		else // old_node->next_ != NULL, update its next node.
		{
			old_node->next_->previous_ = new_node;
		}
		old_node->next_ = new_node;
	}
	++list->length_;
	return list;
}

// Return a list iterator. ListNextIterateNode(iterator) return the next element of the list.
//O(1)
ListIterator *ListGetIterator(List *list, int direction)
{
	ListIterator *iterator;
	if((iterator = Malloc(sizeof(ListIterator))) == NULL)
	{
		return NULL;
	}
	iterator->next_ = (direction == FROM_HEAD_TO_TAIL ? list->head_ : list->tail_);
	iterator->direction_ = direction;
	return iterator;
}

// Return a pointer to the next element of an iterator. It's valid to remove the
// returned element using ListDeleteNode(), but not to remove other elements.
// O(1)
ListNode *ListNextIterateNode(ListIterator *iterator)
{
	ListNode *node = iterator->next_; // Get the next node.
	if(node != NULL)
	{
		iterator->next_ = (iterator->direction_ == FROM_HEAD_TO_TAIL ?
		                   node->next_ : node->previous_);
	}
	return node;
}

// Free the iterator memory.
// O(1)
void ListFreeIterator(ListIterator *iterator)
{
	Free(iterator);
}

// Return a pointer to the node that matches the given key.
// Use the Match method if present, otherwise compare by =.
//O(N)
ListNode *ListSearchKey(List *list, void *key)
{
	ListIterator *iterator = ListGetIterator(list, FROM_HEAD_TO_TAIL);
	ListNode *node;
	while((node = ListNextIterateNode(iterator)) != NULL) // Iterate the list's all nodes.
	{
		// Use list's own match rule and two values are matched.
		if(list->Match != NULL && list->Match(node->value_, key))
		{
			break;
		}
		else if(list->Match == NULL && node->value_ == key) // Use ordinary match =
		{
			break;
		}
	}
	ListFreeIterator(iterator);
	return node; // The matched node or NULL if not matched.
}

// Return list[index] where 0 is the head, 1 is the element next to head and so on.
// O(N)
ListNode *ListIndex(List *list, long index)
{
	ListNode *node = NULL;
	if(0 <= index && index <= list->length_) // In legal range.
	{
		node = list->head_;
		while(node != NULL && index > 0)
		{
			node = node->next_;
			--index;
		}
	}
	return node;
}

// Remove the specified node from the specified list.
// O(1)
void ListDeleteNode(List *list, ListNode *node)
{
	if(node == NULL) // No node to delete.
	{
		return;
	}
	// Update list's nodes
	if(node->previous_)
	{
		node->previous_->next_ = node->next_;
	}
	else
	{
		list->head_ = node->next_;
	}
	if(node->next_)
	{
		node->next_->previous_ = node->previous_;
	}
	else
	{
		list->tail_ = node->previous_;
	}
	if(list->Free) // Use list's private Free function to free this node's value.
	{
		list->Free(node->value_);
	}
	Free(node); // Free this node's memory.
	--list->length_;
}

// Rotate the list removing the tail node and inserting it to the head.
// O(1)
void ListRotate(List *list)
{
	if(list->length_ <= 1)
	{
		return;
	}
	ListNode *node = list->tail_;
	// Remove node from list.
	node->previous_->next_ = NULL;
	list->tail_ = node->previous_;
	// Insert node at the head of the list.
	list->head_->previous_ = node;
	node->next_ = list->head_;
	node->previous_ = NULL;
	list->head_ = node;
}

// Return a pointer to a copy of the original list.
// Use Duplicate method, if present, to copy the node value;
// otherwise the copied node uses(shares) the original node's pointer value.
// O(N)
List *ListDuplicate(List *origin)
{
	List *copy;
	if((copy = ListCreate()) == NULL) // Create new list fail.
	{
		return NULL;
	}
	// Copy data members.
	copy->Duplicate = origin->Duplicate;
	copy->Free = origin->Free;
	copy->Match = origin->Match;
	// Copy origin list's nodes to copy list.
	ListIterator *iterator = ListGetIterator(origin, FROM_HEAD_TO_TAIL);
	ListNode *node;
	void *value;
	while((node = ListNextIterateNode(iterator)) != NULL)
	{
		if(copy->Duplicate) // Use list's private duplicate function.
		{
			if((value = copy->Duplicate(node->value_)) == NULL) // Duplicate error, return.
			{
				ListFree(copy); // Free copy list's nodes and its all memory, and set copy to NULL
				break;
			}
		}
		else
		{
			value = node->value_;
		}
		// Add this value's node as tail node to copy list.
		if(ListAddTailNode(copy, value) == NULL)
		{
			ListFree(copy);
			break;
		}
	}
	ListFreeIterator(iterator);
	return copy; // Maybe NULL.
}
