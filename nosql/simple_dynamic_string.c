#include "simple_dynamic_string.h"

#include <string.h> // strlen(), memcpy(), memset(), memmove(), memcmp()

#include "memory.h"

// Create a new SDS string with the data specified by the 'string' pointer and 'length'.
// O(N)
String SDSNewLength(const void *string, size_t length)
{
	SDS *sds = NULL;
	if(string != NULL)
	{
		// One more byte is allocated for the null-terminated byte '\0'.
		// Malloc does not initialize the memory.
		sds = Malloc(sizeof(SDS) + length + 1);
	}
	else
	{
		// string is NULL, Calloc initializes all the allocated memory to 0.
		sds = Calloc(sizeof(SDS) + length + 1);
	}

	if(sds == NULL) // Allocate memory fail, return NULL.
	{
		return NULL;
	}
	// Initialize the structure's members.
	sds->length_ = length;
	sds->free_ = 0;
	if(string != NULL && length != 0)
	{
		memcpy(sds->data_, string, length); // Copy string.
	}
	sds->data_[length] = '\0'; // The string is always null-terminated.
	return (char*)(sds->data_); // Return the data part of structure.
}

// Create a new SDS string starting from a null terminated C string.
// O(N)
String SDSNew(const void *string)
{
	size_t length = (string == NULL ? 0 : strlen(string));
	return SDSNewLength(string, length); // Delegate work to SDSNewLength.
}

// Create an empty SDS string.
// O(1)
String SDSNewEmpty()
{
	return SDSNewLength("", 0);
}

// Free an SDS string.
// O(1)
void SDSFree(String string)
{
	if(string != NULL) // If string is null, no operation is done.
	{
		Free(string - sizeof(SDS));
	}
}

// Duplicate an SDS string.
// O(N)
String SDSDuplicate(const String string)
{
	return SDSNewLength(string, get_length(string));
}

// Make an SDS string empty(zero length). All the existing buffer is set as free space.
// O(1)
void SDSClear(String string)
{
	SDS *sds = (SDS*)(string - sizeof(SDS));
	// Update data members.
	sds->free_ += sds->length_;
	sds->length_ = 0;
	sds->data_[0] = '\0';
}

// Guarantee that there is at least free `need` bytes in at the end of the SDS string.
// O(1)
String SDSAllocateMemory(String string, size_t need)
{
	SDS *old_sds = (SDS*)(string - sizeof(SDS));
	if(old_sds->free_ >= need) // No need to reallocate memory, just return origin SDS.
	{
		return string;
	}

	size_t new_length = old_sds->length_ + need; // The least need new length.
	if(new_length < SDS_MAX_PREALLOC)
	{
		new_length *= 2; // Double needed length.
	}
	else
	{
		new_length += SDS_MAX_PREALLOC; // Only allocate more 1MB.
	}
	SDS *new_sds = Realloc(old_sds, sizeof(SDS) + new_length + 1);
	if(new_sds == NULL)
	{
		return NULL;
	}
	// Update data members: length_ field remain unchanged, only update free_ field.
	new_sds->free_ = new_length - new_sds->length_;
	return new_sds->data_;
}

// Append the string `append` of 'length' bytes to the end of SDS string `string`.
// After the call, the passed SDS string is no longer valid and all the
// references must be substituted with the new pointer returned by the call.
// O(N)
String SDSAppendLength(String string, const void *append, size_t length)
{
	if((string = SDSAllocateMemory(string, length)) == NULL)
	{
		return NULL; // Can't allocate need memory.
	}

	SDS *sds = (SDS*)(string - sizeof(SDS));
	memcpy(string + sds->length_, append, length); // Append
	// Update data members.
	sds->length_ += length;
	sds->free_ -= length;
	sds->data_[sds->length_] = '\0';
	return string;
}

// Append the c-string `append` to the existing SDS `string`.
// After the call, the modified SDS string is no longer valid and all the
// references must be substituted with the new pointer returned by the call.
// O(N)
String SDSAppend(String string, const char *append)
{
	return SDSAppendLength(string, append, strlen(append));
}

// Append the specified SDS string `append` to the existing SDS string `string`
// O(N)
String SDSAppendSDS(String string, const String append)
{
	return SDSAppendLength(string, append, get_length(append));
}

// Destructively modify the SDS string `string` to hold the string `copy` of `need` bytes.
// O(N)
String SDSCopyLength(String string, const char *copy, size_t need)
{
	SDS *sds = (SDS*)(string - sizeof(SDS)); // Get structure.
	size_t total_length = sds->length_ + sds->free_;
	if(total_length < need) // Current memory is not enough.
	{
		// Try to allocate more memory.
		if((string = SDSAllocateMemory(string, need - sds->length_)) == NULL)
		{
			return NULL;
		}
		// Update the new structure.
		sds = (SDS*)(string - sizeof(SDS));
		total_length = sds->length_ + sds->free_;
	}
	memcpy(string, copy, need); // Copy `copy` to `string`
	string[need] = '\0'; // Always end with null byte.
	sds->length_ = need; // Update data members.
	sds->free_ = total_length - sds->length_;
	return string;
}

// Destructively modify the SDS string `string` to hold the string `copy`
// O(N)
String SDSCopy(String string, const char *copy)
{
	return SDSCopyLength(string, copy, strlen(copy));
}

// Grow the SDS string to have the specified length and fill the added bytes with null.
// If the specified length is smaller than the current length, no operation is performed.
// O(N)
String SDSGrowWithNull(String string, size_t new_length)
{
	SDS *sds = (SDS*)(string - sizeof(SDS));
	size_t old_length = sds->length_;
	if(old_length > new_length) // Don't shrink string.
	{
		return string;
	}
	// May or may not allocate.
	if((string = SDSAllocateMemory(string, new_length - old_length)) == NULL)
	{
		return NULL;
	}
	sds = (SDS*)(string - sizeof(SDS));
	// Fill added bytes with null '\0'; always end with one more null byte.
	memset(string + old_length, 0, new_length - old_length + 1);
	sds->free_ = old_length + sds->free_ - new_length; // old_length remain unchanged.
	sds->length_ = new_length;
	return string;
}

// Modify the string into a substring specified by the `begin` and `end` indexes.
// The interval is inclusive, i.e., [begin, end]
// O(N)
void SDSRange(String string, int begin, int end)
{
	SDS *sds = (SDS*)(string - sizeof(SDS));
	int old_length = sds->length_;
	if(old_length == 0) // No content to move.
	{
		return;
	}
	int new_length = ((0 <= begin && begin < old_length) &&
	                  (0 <= end && end < old_length) &&
	                  begin < end) ? end - begin + 1: 0; // Get the new length.
	if(begin > 0 && new_length > 0) // The range is legal and we need to move.
	{
		memmove(sds->data_, sds->data_ + begin, new_length);
	}
	sds->data_[new_length] = '\0';
	sds->length_ = new_length;
	sds->free_ += old_length - new_length;
}

// Remove the part of the string from left and from right composed just of
// contiguous characters found in 'set', which is a null terminated C string.
// After the call, the modified SDS string is no longer valid and all the
// references must be substituted with the new pointer returned by the call.
// O(N)
String SDSTrim(String string, const char *set)
{
	// Define a map: key is the character, and value(0/!0) indicate
	// whether corresponding character is in set.
	int map_size = (1 << (sizeof(char) * 8)) + 10;
	char map[map_size];
	memset(map, 0, map_size);
	for(const char *key = set; *key != 0; ++key)
	{
		map[*key] = 1;
	}
	SDS *sds = (SDS*)(string - sizeof(SDS));
	char *start = string, *end = string + sds->length_ - 1;
	while(start <= string + sds->length_ - 1 && map[*start] != 0) // O(N).
	{
		++start;
	}
	while(end >= string && map[*end] != 0) // O(N)
	{
		--end;
	}
	int new_length = (start > end) ? 0 : (end - start + 1);
	if(sds->data_ != start) // Need to move content.
	{
		memmove(sds->data_, start, new_length);
	}
	sds->data_[new_length] = '\0';
	sds->free_ += sds->length_ - new_length; // Update data members.
	sds->length_ = new_length;
	return string;
}

// Compare two SDS strings string1 and string2 with memcmp().
// Return: 1 if s1 > s2, -1 if s1 < s2, 0 if s1 = s2.
// If two strings have the same prefix, the longer string is larger.
// O(N)
int SDSCompare(const String string1, const String string2)
{
	size_t length1 = get_length(string1), length2 = get_length(string2);
	size_t min_length = (length1 < length2) ? length1 : length2;
	int result = memcmp(string1, string2, min_length);
	if(result == 0)
	{
		return length1 - length2;
	}
	return result;
}
