#include <skip_list.h>

// TODO: replace `sizeof` by `CAST(int)sizeof`

// Return a pointer to new created skip list node.
SkipListNode *SkipListCreateNode(NosqlObject *object, double score, int level)
{
	// 1. Allocate memory and initialize all memory to 0.
	SkipListNode *node =
	    Calloc(sizeof(SkipListNode) + level * sizeof(struct SkipListLevel));
	// 2. Initialize object_ and score_ fields with arguments,
	node->object_ = object;
	node->score_ = score;
	// Leave level_[] array's all elements and backward_ to 0(NULL).
	return node;
}

// Return a pointer to a new skip list.
// O(1)
SkipList *SkipListCreate()
{
	// 1. Allocate memory.
	SkipList *skip_list = Malloc(sizeof(SkipList));
	// 2. Initialize data members: only create head node.
	skip_list->head_ = SkipListCreateNode(NULL, 0, SKIP_LIST_MAX_LEVEL);
	skip_list->tail_ = NULL;
	skip_list->length_ = 0; // Head node is not included.
	skip_list->level_ = 1;
}

// Decrease this node's object's reference count and free this node's own memory.
void SkipListFreeNode(SkipListNode *node)
{
	// 1. Update reference count of this object.
	DecreaseReferenceCount(node->object_);
	// 2. Free this object safely.
	Free(node);
}

// Free a skip list structure and all its linked nodes. Note that since we use reference count,
// the object stored in each nodes is freed only if this node is the only owner of its object,
// that is, node->object_->reference_count == 1.
// O(N)
void SkipListFree(SkipList *skip_list)
{
	// 1. Store the first node's pointer(`node`) and free head node.
	SkipListNode *node = skip_list->head_->level_[0].forward_, *next_node = NULL;
	Free(skip_list->head_);
	// 2. Traverse the skip list by level[0].forward_ and free each node by SkipListFreeNode.
	while(node)
	{
		next_node = node->level_[0].forward_; // All nodes are linked by level[0] pointer.
		SkipListFreeNode(node);
		node = next_node;
	}
	// 3. Finally, free the skip list structure.
	Free(skip_list);
}

SkipListNode *SkipListInsert(SkipList *skip_list, NosqlObject *object, double score)
{
	// 1. Check whether score is valid.
	assert(isnan(score) == 0);
	// 2. Traverse from the highest level in the skip list to level 0:
	// (1). Store the previous node in level x of insert node in previous[x].
	// (2). Store the total span from head to previous[x] in rank[x].
	SkipListNode *previous[SKIP_LIST_MAX_LEVEL];
	int rank[SKIP_LIST_MAX_LEVEL];
	SkipListNode *node = skip_list->head_, *next_node = NULL;
	for(int level = skip_list->level_ - 1; level >= 0; --level)
	{
		// Store rank that is crossed to reach the insert position.
		rank[level] = ((level == skip_list->level_ - 1) ? 0 : rank[level + 1]);
		next_node = node->level_[level].forward_; // The next node in this level.
		while(next_node != NULL && (next_node->score_ < score ||
		                            (next_node->score_ == score &&
		                             CompareStringObjects(next_node->object_, object) < 0)))
		{
			// We can insert new node after next_node. Update rank and node:
			rank[level] += node->level_[level].span_;
			node = next_node;
			next_node = next_node->level_[level].forward_; // Test the next node in this level.
		}
		// previous[level] is the just previous node of insert node in level `level`.
		previous[level] = node;
	}
	// 3. Randomly get the level number of insert node and create the new node.
	int new_level = SkipListRandomLevel();
	SkipListNode *new_node = SkipListCreateNode(object, score, new_level);
	// 4. If new_level is greater than skip list's level: update list's data member
	// and set rank, previous array for the added levels.
	if(new_level > skip_list->level_)
	{
		for(int level = skip_list->level_; level < new_level; ++level)
		{
			skip_list->level_[level].span_ = skip_list->length_;
			rank[level] = 0;
			previous[level] = skip_list->head_;
		}
		skip_list->level_ = new_level;
	}
	// 5. Insert new node: set forward_, span_ and backward_.
	for(int level = 0; level < new_level; ++level)
	{
		// Set level_[level].forward_:
		new_node->level_[level].forward_ = previous[level]->level_[level].forward_;
		previous[level]->level_[level].forward_ = new_node;
		// Set level_[level].span_:
		// Before insert: previous[level] -(span1)> node.
		// After insert: previous[level] -(span2)> new_node -(span3)> node
		// We have the formula: span1 + 1 = span2 + span3.
		// 1.	span1 is the old value of span2: previous[level]->level_[level].span_
		// 2.	span2 = (rank[0] + 1) - rank[level]. rank[level] is the sum of all span from head
		//		to previous[level], so `rank[0]` is the total span from head to previous[0]. Since
		//		all nodes have level[0] and we insert the new node after previous[0] in level[0],
		//		so `rank[0] + 1` is the sum of all span from head to new_node. Thus, the span
		//		between previous[level] and new_node is (rank[0] + 1) - rank[level].
		// 3.	span3	= span1 + 1 - span2 = span1 + 1 - rank[0] - 1 + rank[level]
		//					= span1 - rank[0] + rank[level]
		//					= previous[level]->level_[level].span_ - rank[0] + rank[level]
		// We should first use span1 to calculate span3 and then update span1 to span2.
		new_node->level_[level].span_ = previous[level]->level_[level].span_ - rank[0] + rank[level];
		previous[level]->levle_[level].span_ = (rank[0] + 1) - rank[level];
	}
	// If new_level is less than the skip list's level, then for the previous[x] that x is
	// greater than new_level, since new_node doesn't have x level, we don't link
	// previous[x] with new_node, only increase previous[x]->level_[x].span_ by one
	// to indicate that a new node is inserted between previous[x] and its next node.
	for(int level = new_level; level < skip_list->level_; ++level)
	{
		++previous[level]->level_[level].span_;
	}
	// Set backward pointer for new_node and its next node.
	// Two conditions: new_node is the first node or not.
	new_node->backward_ = ((previous[0] == skip_list->head_) ? NULL // The first node.
	                       : previous[0]); // Not the first node.
	// Two conditions: new_node is the last node or not.
	if(new_node->level[0].forward_) // Not the last node.
	{
		new_node->level_[0].forward_->backward_ = new_node;
	}
	else // The last node.
	{
		skip_list->tail_ = new_node;
	}
	// 6. Finally, update skip list's length.
	++skip_list->length_;
	return new_node;
}
