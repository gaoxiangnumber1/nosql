#ifndef NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_ // PROJECT_PATH_FILE_H_
#define NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_

#ifndef CAST
#define CAST(type) (type)
#endif

#define SDS_MAX_PREALLOC (1024*1024) // The maximum bytes pre-allocate, 1MB

typedef char* String; // Type alias.

typedef struct SimpleDynamicString // Structure that stores string object.
{
	int length_; // Space have used in bytes.
	int free_; // Space free in bytes.
	char data_[]; // Store string content.
} SDS;

// Return the size of space has used in bytes, i.e., the member length_.
// O(1)
static inline int get_length(const String str)
{
	SDS *sds = CAST(SDS*)(str - sizeof(SDS));
	return sds->length_;
}

// Return the size of free space in bytes, i.e., the member free_.
// O(1)
static inline int get_free(const String str)
{
	SDS *sds = CAST(SDS*)(str - sizeof(SDS));
	return sds->free_;
}

// Create a new SDS string with the data specified by the 'string' pointer and 'length'.
String SDSNewLength(const void *string, int length);
// Create a new SDS string starting from a null terminated C string.
String SDSNew(const void *string);
// Create an empty SDS string.
String SDSNewEmpty();
// Free an SDS string."
void SDSFree(String string);
// Duplicate an SDS string.
String SDSDuplicate(const String string);
// Make an SDS string empty(zero length).
void SDSClear(String string);
// Guarantee that there is at least free `need` bytes in at the end of the SDS string.
String SDSAllocateMemory(String string, int need);
// Append the string `append` of 'length' bytes to the end of SDS string `string`.
String SDSAppendLength(String string, const void *append, int length);
// Append the c-string `append` to the existing SDS `string`.
String SDSAppend(String string, const char *append);
// Append the specified SDS string `append` to the existing SDS string `string`
String SDSAppendSDS(String string, const String append);
// Destructively modify the SDS string `string` to hold the string `copy` of `need` bytes.
String SDSCopyLength(String string, const char *copy, int need);
// Destructively modify the SDS string `string` to hold the string `copy`
String SDSCopy(String string, const char *copy);
// Grow the SDS string to have the specified length and fill the added bytes with null.
String SDSGrowWithNull(String string, int new_length);
// Modify the string into a substring specified by the `begin` and `end` indexes.
void SDSRange(String string, int begin, int end);
// Remove the part of the string from left and from right composed just of
// contiguous characters found in 'set', which is a null terminated C string.
String SDSTrim(String string, const char *set);
// Compare two SDS strings string1 and string2 with memcmp().
int SDSCompare(const String string1, const String string2);

#endif // NOSQL_SRC_SIMPLE_DYNAMIC_STRING_H_
