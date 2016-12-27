#ifndef NOSQL_SRC_NOSQL_H_

// Actual Nosql Object
#define NOSQL_LRU_BITS 24
// Max value of obj->lru_
#define NOSQL_LRU_CLOCK_MAX ((1<<NOSQL_LRU_BITS) - 1)
// LRU clock resolution in ms
#define NOSQL_LRU_CLOCK_RESOLUTION 1000

typedef struct NosqlObject
{
	unsigned type_ : 4;
	unsigned encoding_ : 4;
	unsigned lru_ : NOSQL_LRU_BITS; // lru time(relative to server.lruclock)
	int reference_count_;
	void *ptr_;
} NosqlObject;
#endif // NOSQL_SRC_NOSQL_H_
