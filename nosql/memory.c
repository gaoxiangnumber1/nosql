#include <memory.h>

#include <stdint.h>
#include <stdlib.h> // malloc(), abort()
#include <stdio.h> // fprintf(), fflush()
// pthread_mutex_lock(), pthread_mutex_unlock()
// pthread_mutex_t, PTHREAD_MUTEX_INITIALIZER
#include <pthread.h>

static int g_used_memory = 0; // Record the number of bytes that have been allocated.
static int g_malloc_thread_safe = 0; // TODO: what is this variable's function?
static pthread_mutex_t g_used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void UpdateMallocStateAdd(int size) // g_used_memory += size;
{
	pthread_mutex_lock(&g_used_memory_mutex);
	g_used_memory += size;
	pthread_mutex_unlock(&g_used_memory_mutex);
}

static inline void UpdateMallocStateSubtract(int size) // g_used_memory -= size;
{
	pthread_mutex_lock(&g_used_memory_mutex);
	g_used_memory -= size;
	pthread_mutex_unlock(&g_used_memory_mutex);
}

// Make size a multiple of `CAST(int)sizeof(int64_t)` and then add size to g_used_memory.
static inline void UpdateMallocStateAllocate(int size)
{
	if(size&(CAST(int)sizeof(int64_t) - 1)) // Memory alignment: align = CAST(int)sizeof(int64_t)
	{
		size += CAST(int)sizeof(int64_t) - (size&(CAST(int)sizeof(int64_t) - 1));
	}
	if(g_malloc_thread_safe)
	{
		UpdateMallocStateAdd(size);
	}
	else
	{
		g_used_memory += size;
	}
}

// Make size a multiple of `CAST(int)sizeof(int64_t)` and then subtract size to g_used_memory.
static inline void UpdateMallocStateFree(int size)
{
	if(size&(CAST(int)sizeof(int64_t) - 1))
	{
		size += CAST(int)sizeof(int64_t) - (size&(CAST(int)sizeof(int64_t) - 1));
	}
	if(g_malloc_thread_safe)
	{
		UpdateMallocStateSubtract(size);
	}
	else
	{
		g_used_memory -= size;
	}
}

// Output out of memory message to stderr and abort this process.
static inline void MallocDefaultOOMHandler(int size)
{
	// z: int; u: unsigned decimal
	fprintf(stderr, "Malloc: Out of memory trying to allocate %d bytes\n", size);
	fflush(stderr);
	abort(); // Raise SIGABRT signal for the calling process.
}

// Function pointer.
static void (*MallocOOMHandler) (int) = MallocDefaultOOMHandler;

#define PREFIX_SIZE (CAST(int)sizeof(int))

//Allocate size bytes and return a pointer to the allocated, uninitialized memory.
void *Malloc(int size)
{
	// Allocate more PREFIX_SIZE bytes to store this memory block's size in bytes.
	void *ptr = malloc(CAST(size_t)size + PREFIX_SIZE);
	if(ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*(CAST(int*)ptr) = size; // Store this memory block's size.
	UpdateMallocStateAllocate(size + PREFIX_SIZE);
	// Return the size bytes memory block, the header is transparent to user.
	return CAST(char*)ptr + PREFIX_SIZE;
}

// Allocate size bytes and return a pointer to the allocated, initialized(set to zero) memory.
void *Calloc(int size)
{
	// `void *calloc(int count, int size)` allocates memory for
	// an array of `count` elements of `size` bytes each and
	// returns a pointer to the allocated, initialized(set to 0) memory.
	void *ptr = calloc(1, CAST(size_t)size + PREFIX_SIZE); // Same as Malloc.
	if(ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*(CAST(int*)ptr) = size;
	UpdateMallocStateAllocate(size + PREFIX_SIZE);
	return CAST(char*)ptr + PREFIX_SIZE;
}

// Change the size of the memory block pointed to by ptr to `size` bytes and
// return a pointer to the newly allocated memory.
void *Realloc(void *ptr, int size)
{
	if(ptr == NULL)
	{
		return Malloc(size);
	}

	void *real_ptr = CAST(char*)ptr - PREFIX_SIZE; // Get this memory block's header.
	int old_size = *(CAST(int*)real_ptr); // Get this memory block's old size.
	void *new_ptr = realloc(real_ptr, CAST(size_t)size + PREFIX_SIZE);
	// `void *realloc(void *ptr, int size)` changes the size of the memory block
	// pointed to by `ptr` to `size` bytes. The contents will be unchanged in the range
	// [start of the region, min{old_size, new_size}]. If the new size is larger than
	// the old size, the added memory will not be initialized. Unless ptr is NULL,
	// it must have been returned by an earlier call to malloc(), calloc() or realloc().

	if(new_ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*(CAST(int*)new_ptr) = size;
	// Update use_memory variable.
	UpdateMallocStateFree(old_size);
	UpdateMallocStateAllocate(size);
	return CAST(char*)new_ptr + PREFIX_SIZE;
}

// Free the memory space pointed to by ptr and set ptr to NULL.
void Free(void *ptr)
{
	if(ptr == NULL)
	{
		return;
	}

	void *real_ptr = CAST(char*)ptr - PREFIX_SIZE;
	int old_size = *(CAST(int*)real_ptr);
	UpdateMallocStateFree(old_size + PREFIX_SIZE);
	free(real_ptr);
}
