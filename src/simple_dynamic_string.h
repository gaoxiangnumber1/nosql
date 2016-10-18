#ifndef NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_ // PROJECT_PATH_FILE_H_
#define NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_

#define SDS_MAX_PREALLOC (1024*1024) // The maximum bytes pre-allocate, 1MB

typedef char* String; // Type alias.

struct SimpleDynamicString // Structure that stores string object.
{
	int used_; // Space have used in bytes.
	int free_; // Space free in bytes.
	char data_[]; // Store string content.
};

// Return the size of space has used in bytes, i.e., the member used_. O(1)
static inline size_t get_used(const String str)
{
	struct SimpleDynamicString *sds = (void*)(str - sizeof(struct SimpleDynamicString));
	return sds->used_;
}

// Return the size of free space in bytes, i.e., the member free_. O(1)
static inline size_t get_free(const String str)
{
	struct  SimpleDynamicString *sds = (void*)(str - sizeof(struct SimpleDynamicString));
	return sds->free_;
}

#endif // NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_
