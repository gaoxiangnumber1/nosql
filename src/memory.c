#include <stdlib.h> // malloc(), abort()
#include <stdio.h> // fprintf(), fflush()
// pthread_mutex_lock(), pthread_mutex_unlock()
// pthread_mutex_t, PTHREAD_MUTEX_INITIALIZER
#include <pthread.h>

static size_t g_used_memory = 0; // Record the number of bytes that have been allocated.
static int g_malloc_thread_safe = 0; // TODO: what is this variable's function?
static pthread_mutex_t g_used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void UpdateMallocStateAdd(size_t size) // g_used_memory += size;
{
	pthread_mutex_lock(&g_used_memory_mutex);
	g_used_memory += size;
	pthread_mutex_unlock(&g_used_memory_mutex);
}

static inline void UpdateMallocStateSubtract(size_t size) // g_used_memory -= size;
{
	pthread_mutex_lock(&g_used_memory_mutex);
	g_used_memory -= size;
	pthread_mutex_unlock(&g_used_memory_mutex);
}

// Make size a multiple of `sizeof(long)` and then add size to g_used_memory.
static inline void UpdateMallocStateAllocate(size_t size)
{
	if(size&(sizeof(long) - 1)) // Memory alignment: align = sizeof(long)
	{
		size += sizeof(long) - (size&(sizeof(long) - 1));
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

// Make size a multiple of `sizeof(long)` and then subtract size to g_used_memory.
static inline void UpdateMallocStateFree(size_t size)
{
	if(size&(sizeof(long) - 1))
	{
		size += sizeof(long) - (size&(sizeof(long) - 1));
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
static inline void MallocDefaultOOMHandler(size_t size)
{
	// z: size_t; u: unsigned decimal
	fprintf(stderr, "Malloc: Out of memory trying to allocate %zu bytes\n", size);
	fflush(stderr);
	abort(); // Raise SIGABRT signal for the calling process.
}

// Function pointer.
static void (*MallocOOMHandler) (size_t) = MallocDefaultOOMHandler;

#define PREFIX_SIZE (sizeof(size_t))

//Allocate size bytes and return a pointer to the allocated, uninitialized memory.
void *Malloc(size_t size)
{
	// Allocate more PREFIX_SIZE bytes to store this memory block's size in bytes.
	void *ptr = malloc(size + PREFIX_SIZE);
	if(ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*((size_t*)ptr) = size; // Store this memory block's size.
	UpdateMallocStateAllocate(size + PREFIX_SIZE);
	// Return the size bytes memory block, the header is transparent to user.
	return (char*)ptr + PREFIX_SIZE;
}

// Allocate size bytes and return a pointer to the allocated, initialized(set to zero) memory.
void *Calloc(size_t size)
{
	// `void *calloc(size_t count, size_t size)` allocates memory for
	// an array of `count` elements of `size` bytes each and
	// returns a pointer to the allocated, initialized(set to 0) memory.
	void *ptr = calloc(1, size + PREFIX_SIZE); // Same as Malloc.
	if(ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*((size_t*)ptr) = size;
	UpdateMallocStateAllocate(size + PREFIX_SIZE);
	return (char*)ptr + PREFIX_SIZE;
}

// Change the size of the memory block pointed to by ptr to `size` bytes and
// return a pointer to the newly allocated memory.
void *Realloc(void *ptr, size_t size)
{
	if(ptr == NULL)
	{
		return Malloc(size);
	}

	void *real_ptr = (char*)ptr - PREFIX_SIZE; // Get this memory block's header.
	size_t old_size = *((size_t*)real_ptr); // Get this memory block's old size.
	void *new_ptr = realloc(real_ptr, size + PREFIX_SIZE);
	// `void *realloc(void *ptr, size_t size)` changes the size of the memory block
	// pointed to by `ptr` to `size` bytes. The contents will be unchanged in the range
	// [start of the region, min{old_size, new_size}]. If the new size is larger than
	// the old size, the added memory will not be initialized. Unless ptr is NULL,
	// it must have been returned by an earlier call to malloc(), calloc() or realloc().

	if(new_ptr == NULL)
	{
		MallocOOMHandler(size);
	}
	*((size_t*)new_ptr) = size;
	// Update use_memory variable.
	UpdateMallocStateFree(old_size);
	UpdateMallocStateAllocate(size);
	return (char*)new_ptr + PREFIX_SIZE;
}

// Free the memory space pointed to by ptr and set ptr to NULL.
void Free(void *ptr)
{
	if(ptr == NULL)
	{
		return;
	}

	void *real_ptr = (char*)ptr - PREFIX_SIZE;
	size_t old_size = *((size_t*)real_ptr);
	UpdateMallocStateFree(old_size + PREFIX_SIZE);
	free(real_ptr);
}
