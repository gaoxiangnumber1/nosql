#ifndef NOSQL_TEST_TEST_H_
#define NOSQL_TEST_TEST_H_

#include <stdlib.h> // exit()

int __test_count = 0;
int __failed_tests = 0;

#define TEST_CONDITION(description,condition) \
++__test_count; \
printf("%d - %s: ", __test_count, description); \
if(condition) \
{ \
	printf("Passed\n"); \
} \
else \
{ \
	printf("Failed\n"); \
	++__failed_tests; \
}

#define TEST_REPORT() \
printf("%d Tests, %d Passed, %d Failed\n", \
       __test_count, __test_count - __failed_tests, __failed_tests); \
if(__failed_tests) \
{ \
	printf("Error: we failed tests here.\n"); \
	exit(1); \
}

#endif // NOSQL_TEST_TEST_H_
