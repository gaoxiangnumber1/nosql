#include <stdio.h> // printf()
#include <string.h> // strlen(), memcmp()

#include <double_linked_list.h>
#include "test.h"

int g_call_duplicate_count = 0, g_call_free_count = 0, g_call_match_count = 0;

void *DuplicateString(void *value)
{
	int length = strlen((char*)value);
	char *string = malloc(length);
	memcpy(string, value, length); // Copy
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

void *MatchString(void *value1, void *value2) // Return NULL if not match.
{
	++g_call_match_count;
	int length1 = strlen((char*)value1), length2 = strlen((char*)value2);
	if(length1 == length2 && memcmp(value1, value2, length1) == 0)
	{
		return (void*)1;
	}
	return (void*)0;
}

int main(void)
{
	List *list1 = ListCreate();
	TEST_CONDITION("List list1 = ListCreate();", ListHeadNode(list1) == NULL &&
	               ListLength(list1) == 0 && ListGetDuplicateMethod(list1) == NULL);

	ListSetDuplicateMethod(list1, DuplicateString);
	ListSetFreeMethod(list1, FreeString);
	ListSetMatchMethod(list1, MatchString);

	list1 = ListAddHeadNode(list1, "xiang");
	list1 = ListAddHeadNode(list1, "gao");
	TEST_CONDITION("list1 = ListAddHeadNode(list1, \"xiang;gao\");",
	               ListLength(list1) == 2 &&
	               memcmp(ListNodeValue(ListHeadNode(list1)), "gao\0", 4) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list1)), "xiang\0", 6) == 0);

	ListDeleteNode(list1, ListIndex(list1, 0));
	ListDeleteNode(list1, ListIndex(list1, 0));
	TEST_CONDITION("ListDeleteNode(list1, ListIndex(list1, 0)); * 2",
	               g_call_free_count == 2 && ListLength(list1) == 0);

	list1 = ListAddTailNode(list1, "gao");
	list1 = ListAddTailNode(list1, "xiang");
	TEST_CONDITION("list1 = ListAddTailNode(list1, \"xiang;gao\");",
	               ListLength(list1) == 2 &&
	               memcmp(ListNodeValue(ListHeadNode(list1)), "gao\0", 4) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list1)), "xiang\0", 6) == 0);

	list1 = ListInsertNode(list1, ListIndex(list1, 0), "hello", 0);
	list1 = ListInsertNode(list1, ListIndex(list1, 1), "I'm", 0);
	list1 = ListInsertNode(list1, ListIndex(list1, ListLength(list1) - 1), "one", 1);
	list1 = ListInsertNode(list1, ListIndex(list1, ListLength(list1) - 2), "number", 1);
	TEST_CONDITION("ListInsertNode(list1, ListIndex(list1, 0), \"...\", 0); * 4",
	               ListLength(list1) == 6 &&
	               memcmp(ListNodeValue(ListHeadNode(list1)), "hello\0", 6) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list1)), "one\0", 4) == 0);

	list1 = ListInsertNode(list1, ListSearchKey(list1, "xiang"), "To", 1);
	list1 = ListInsertNode(list1, ListSearchKey(list1, "number"), "be", 0);
	TEST_CONDITION("ListInsertNode(list1, ListSearchKey(list1, \"xiang\"), \"To\", 1); * 2",
	               ListLength(list1) == 8 &&
	               g_call_match_count == 10 &&
	               memcmp(ListNodeValue(ListIndex(list1, 3)), "xiang\0", 6) == 0 &&
	               memcmp(ListNodeValue(ListIndex(list1, 5)), "be\0", 3) == 0);

	ListRotate(list1);
	ListRotate(list1);
	TEST_CONDITION("ListRotate(list1); * 2",
	               ListLength(list1) == 8 &&
	               memcmp(ListNodeValue(ListHeadNode(list1)), "number\0", 7) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list1)), "be\0", 3) == 0);

	List *list2 = ListDuplicate(list1);
	TEST_CONDITION("ListDuplicate(list1);",
	               ListLength(list2) == 8 &&
	               g_call_duplicate_count == 8 &&
	               memcmp(ListNodeValue(ListHeadNode(list2)), "number\0", 7) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list2)), "be\0", 3) == 0);

	ListDeleteNode(list1, ListNextNode(ListHeadNode(list1)));
	ListDeleteNode(list1, ListHeadNode(list1));
	ListDeleteNode(list1, ListPreviousNode(ListTailNode(list1)));
	ListDeleteNode(list1, ListTailNode(list1));
	TEST_CONDITION("ListDeleteNode(list1, ListNextNode(ListHeadNode(list1))); * 4",
	               g_call_free_count == 6 &&
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
	TEST_CONDITION("ListDeleteNode(list1, ListHeadNode(list1)); * 6",
	               g_call_free_count == 10 &&
	               ListLength(list1) == 0 &&
	               ListLength(list2) == 8 &&
	               memcmp(ListNodeValue(ListHeadNode(list2)), "number\0", 7) == 0 &&
	               memcmp(ListNodeValue(ListTailNode(list2)), "be\0", 3) == 0);

	ListFree(list1);
	ListFree(list2);
	TEST_CONDITION("ListFree(list1); * 2", g_call_free_count == 18);

	TEST_REPORT();

	return 0;
}
