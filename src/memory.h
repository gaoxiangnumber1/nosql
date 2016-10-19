#ifndef NOSQL_SRC_MEMORY_H_
#define NOSQL_SRC_MEMORY_H_

// Allocate size bytes and return a pointer to the allocated, uninitialized memory.
void *Malloc(size_t size);
// Allocate size bytes and return a pointer to the allocated, initialized(set to zero) memory.
void *Calloc(size_t size);
// Change the size of the memory block pointed to by ptr to `size` bytes and
// return a pointer to the newly allocated memory
void Realloc(void *ptr, size_t size);

#endif // NOSQL_SRC_MEMORY_H_
