#ifndef NOSQL_SRC_SKIP_LIST_H_
#define NOSQL_SRC_SKIP_LIST_H_

#ifndef CAST
#define CAST(type) (type)
#endif

#include <nosql.h>

typedef struct SkipListNode
{
	NosqlObject *object_; // Pointer to its stored object.
	double score_; // Object's corresponding score.
	struct SkipListLevel // A structure that combines a forwarding pointer and its span.
	{
		// Point to the next node and the distance between them is span_.
		struct SkipListNode *forward_;
		int span_; // The number of links between this node and the forward_ node.
	} level_[]; // An array.
	struct SkipListNode *backward_; // Point to the just backward node.
} SkipListNode;

typedef struct SkipList
{
	struct SkipListNode *head_, *tail_;
	int length_; // The number of nodes in skip list.
	int level_; // The number of levels in the node which has the most levels in the skip list.
} SkipList;

#define SKIP_LIST_MAX_LEVEL 32 // Should be enough for 2^32 elements

#endif // NOSQL_SRC_SKIP_LIST_H_
