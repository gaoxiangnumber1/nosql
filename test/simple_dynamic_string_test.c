#include <stdio.h> // printf()
#include <string.h> // strlen(), memcmp()
#include <assert.h>

#include <simple_dynamic_string.h>

int main(void)
{
	String s1, s2;

	s1 = SDSNewLength(NULL, 5);
	assert(get_length(s1) == 5 && get_free(s1) == 0 && memcmp(s1, "\0\0\0\0\0\0", 6) == 0);

	SDSFree(s1);
	s1 = SDSNewLength("xiang", 0);
	assert(get_length(s1) == 0 && get_free(s1) == 0 && memcmp(s1, "\0", 1) == 0);

	SDSFree(s1);
	s1 = SDSNewLength("xiang", 3);
	assert(get_length(s1) == 3 && get_free(s1) == 0 && memcmp(s1, "xia\0", 4) == 0);

	SDSFree(s1);
	s1 = SDSNewLength("xiang", 5);
	assert(get_length(s1) == 5 && get_free(s1) == 0 && memcmp(s1, "xiang\0", 6) == 0);

	SDSFree(s1);
	s1 = SDSNewLength("xiang", 6);
	assert(get_length(s1) == 6 && get_free(s1) == 0 && memcmp(s1, "xiang\0\0", 7) == 0);

	SDSFree(s1);
	s1 = SDSNew("gaoxiang");
	assert(get_length(s1) == 8 && get_free(s1) == 0 && memcmp(s1, "gaoxiang\0", 9) == 0);

	SDSFree(s1);
	s1 = SDSNewEmpty();
	assert(get_length(s1) == 0 && get_free(s1) == 0 && memcmp(s1, "\0", 1) == 0);

	SDSFree(s1);
	s1 = SDSNew("number");
	s2 = SDSDuplicate(s1);
	assert(get_length(s2) == 6 && get_free(s2) == 0 && memcmp(s2, "number\0", 7) == 0);

	SDSClear(s2);
	assert(get_length(s2) == 0 && get_free(s2) == 6 && memcmp(s2, "\0umber\0", 7) == 0);

	s1 = SDSAppendLength(s1, "one", 3);
	assert(get_length(s1) == 9 && get_free(s1) == 9 && memcmp(s1, "numberone\0", 10) == 0);

	SDSFree(s2);
	s2 = SDSNew("gaoxiang");
	s1 = SDSAppendSDS(s1, s2);
	assert(get_length(s1) == 17 && get_free(s1) == 1 && memcmp(s1, "numberonegaoxiang\0", 18) == 0);

	s1 = SDSAppend(s1, "!!!");
	assert(get_length(s1) == 20 && get_free(s1) == 20 && memcmp(s1, "numberonegaoxiang!!!\0", 21) == 0);

	s1 = SDSCopyLength(s1, "xiang", 6);
	assert(get_length(s1) == 6 && get_free(s1) == 34 && memcmp(s1, "xiang\0\0negaoxiang!!!\0", 21) == 0);

	s1 = SDSCopy(s1, "gao");
	assert(get_length(s1) == 3 && get_free(s1) == 37 && memcmp(s1, "gao\0g\0\0negaoxiang!!!\0", 21) == 0);

	s1 = SDSGrowWithNull(s1, 10);
	assert(get_length(s1) == 10 && get_free(s1) == 30 && memcmp(s1, "gao\0\0\0\0\0\0\0\0oxiang!!!\0", 21) == 0);

	SDSRange(s1, 5, 9);
	assert(get_length(s1) == 5 && get_free(s1) == 35 && memcmp(s1, "\0\0\0\0\0\0\0\0\0\0\0oxiang!!!\0", 21) == 0);

	s1 = SDSCopy(s1, "I am gao xiang. To be number one!");
	assert(get_length(s1) == 33 && get_free(s1) == 7 && memcmp(s1, "I am gao xiang. To be number one!\0", 34) == 0);

	s1 = SDSTrim(s1, "Iabcde !");
	assert(get_length(s1) == 28 && get_free(s1) == 12 && memcmp(s1, "m gao xiang. To be number on\0one!\0", 34) == 0);

	int value = SDSCompare(s1, s2);
	assert(value > 0);

	printf("All passed! Come on!\n");

	return 0;
}
