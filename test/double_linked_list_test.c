#include <stdio.h> // printf()
#include <string.h> // CAST(int)strlen(), memcmp()
#include <stdlib.h>
#include <assert.h>

#include <double_linked_list.h>

int g_call_duplicate_count = 0, g_call_free_count = 0, g_call_match_count = 0;

void *DuplicateString(const void *value)
{
	int length = CAST(int)strlen(CAST(const char*)value);
	char *string = CAST(char*)malloc(CAST(size_t)length);
	memcpy(string, value, CAST(size_t)length); // Copy
	++g_call_duplicate_count;
	return string;
}

void *FreeString(void *value)
{
	if(value != NULL)
	{
		// FIXME: how to distinguish between the string literal and the string got by malloc?
		// If we use free(3) to free a pointer that is a string literal, we'll get Segmentation fault.
		//free(value);
		//*(char*)value = 0;
		value = NULL;
		++g_call_free_count;
	}
	return NULL;
}

void *MatchString(const void *value1, const void *value2) // Return NULL if not match.
{
	++g_call_match_count;
	int length1 = CAST(int)strlen(CAST(const char*)value1),
	    length2 = CAST(int)strlen(CAST(const char*)value2);
	if(length1 == length2 && memcmp(value1, value2, CAST(size_t)length1) == 0)
	{
		return CAST(void*)1;
	}
	return CAST(void*)0;
}

int main(void)
{
	List *list1 = ListCreate();
	assert(ListHeadNode(list1) == NULL && ListLength(list1) == 0 &&
	       ListGetDuplicateMethod(list1) == NULL);

	ListSetDuplicateMethod(list1, DuplicateString);
	ListSetFreeMethod(list1, FreeString);
	ListSetMatchMethod(list1, MatchString);

	list1 = ListAddHeadNode(list1, "xiang");
	list1 = ListAddHeadNode(list1, "gao");
	assert(ListLength(list1) == 2 &&
	       memcmp(ListNodeValue(ListHeadNode(list1)), "gao\0", 4) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list1)), "xiang\0", 6) == 0);

	ListDeleteNode(list1, ListIndex(list1, 0));
	ListDeleteNode(list1, ListIndex(list1, 0));
	assert(g_call_free_count == 2 && ListLength(list1) == 0);

	list1 = ListAddTailNode(list1, "gao");
	list1 = ListAddTailNode(list1, "xiang");
	assert(ListLength(list1) == 2 &&
	       memcmp(ListNodeValue(ListHeadNode(list1)), "gao\0", 4) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list1)), "xiang\0", 6) == 0);

	list1 = ListInsertNode(list1, ListIndex(list1, 0), "hello", 0);
	list1 = ListInsertNode(list1, ListIndex(list1, 1), "I'm", 0);
	list1 = ListInsertNode(list1, ListIndex(list1, ListLength(list1) - 1), "one", 1);
	list1 = ListInsertNode(list1, ListIndex(list1, ListLength(list1) - 2), "number", 1);
	assert(ListLength(list1) == 6 &&
	       memcmp(ListNodeValue(ListHeadNode(list1)), "hello\0", 6) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list1)), "one\0", 4) == 0);

	list1 = ListInsertNode(list1, ListSearchKey(list1, "xiang"), "To", 1);
	list1 = ListInsertNode(list1, ListSearchKey(list1, "number"), "be", 0);
	assert(ListLength(list1) == 8 &&
	       g_call_match_count == 10 &&
	       memcmp(ListNodeValue(ListIndex(list1, 3)), "xiang\0", 6) == 0 &&
	       memcmp(ListNodeValue(ListIndex(list1, 5)), "be\0", 3) == 0);

	ListRotate(list1);
	ListRotate(list1);
	assert(ListLength(list1) == 8 &&
	       memcmp(ListNodeValue(ListHeadNode(list1)), "number\0", 7) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list1)), "be\0", 3) == 0);

	List *list2 = ListDuplicate(list1);
	assert(ListLength(list2) == 8 &&
	       g_call_duplicate_count == 8 &&
	       memcmp(ListNodeValue(ListHeadNode(list2)), "number\0", 7) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list2)), "be\0", 3) == 0);

	ListDeleteNode(list1, ListNextNode(ListHeadNode(list1)));
	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListPreviousNode(ListTailNode(list1)));
	ListDeleteNode(list1, ListTailNode(list1));
	assert(g_call_free_count == 6 &&
	       ListLength(list1) == 4 &&
	       memcmp(ListNodeValue(ListHeadNode(list1)), "hello\0", 6) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list1)), "xiang\0", 6) == 0 &&
	       ListLength(list2) == 8 &&
	       memcmp(ListNodeValue(ListHeadNode(list2)), "number\0", 7) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list2)), "be\0", 3) == 0);

	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListHeadNode(list1)); // Two more times.
	ListDeleteNode(list1, ListHeadNode(list1));
	assert(g_call_free_count == 10 &&
	       ListLength(list1) == 0 &&
	       ListLength(list2) == 8 &&
	       memcmp(ListNodeValue(ListHeadNode(list2)), "number\0", 7) == 0 &&
	       memcmp(ListNodeValue(ListTailNode(list2)), "be\0", 3) == 0);

	ListFree(list1);
	ListFree(list2);
	assert(g_call_free_count == 18);

	printf("All Passed! Come On!\n");

	return 0;
}
