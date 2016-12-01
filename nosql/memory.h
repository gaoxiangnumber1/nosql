#ifndef NOSQL_SRC_MEMORY_H_
#define NOSQL_SRC_MEMORY_H_

#ifndef CAST
#define CAST(type) (type)
#endif

//Allocate size bytes and return a pointer to the allocated, uninitialized memory.
void *Malloc(int size);
// Allocate size bytes and return a pointer to the allocated, initialized(set to zero) memory.
void *Calloc(int size);
// Change the size of the memory block pointed to by ptr to `size` bytes and
// return a pointer to the newly allocated memory.
void *Realloc(void *ptr, int size);
// Free the memory space pointed to by ptr and set ptr to NULL.
void Free(void *ptr);

#endif // NOSQL_SRC_MEMORY_H_
