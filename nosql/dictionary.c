#include "dictionary.h"

#include "memory.h"

// We can use DictionaryEnableResize() or DictionaryDisableResize() to enable/disable
// resizing of the hash table. This is important as we use COW and don't want to move
// too much memory when there is a child performing saving operations.

// When the ratio of elements_number/buckets_number is greater than
// dictionary_force_resize_ratio, a hash table is allowed to grow regardless of
// the value of dictionary_can_resize.

static int dictionary_can_resize = 1; // Indicate whether we are allowed to resize the hash table.
// The safe threshold for the ratio between elements/bucket. When the actual ratio
// is over this threshold, we force resize.
static unsigned int dictionary_force_resize_ratio = 5;

// Enable resizing of the hash table.
// O(1)
void DictionaryEnableResize()
{
	dictionary_can_resize = 1;
}

// Disable resizing of the hash table.
// O(1)
void DictionaryDisableResize()
{
	dictionary_can_resize = 0;
}

// Reset hash table structure's all data members to zero.
// O(1)
void DictionaryResetHashTable(HashTable *hash_table)
{
	hash_table->slot_ = NULL;
	hash_table->size_ = hash_table->size_mask_ = hash_table->element_number_ = 0;
}

// Return a pointer to new dictionary created specified by type and argument.
// O(1)
Dictionary *DictionaryCreate(HashTableType *type, void *argument)
{
	// Allocate memory only for Dictionary structure.
	Dictionary *dictionary = Malloc(sizeof(Dictionary));
	// Initialize two hash tables without allocating memory. Allocate memory for them
	// when the first time call DictionaryExpandIfNeeded().
	DictionaryResetHashTable(&dictionary->hash_table_[0]);
	DictionaryResetHashTable(&dictionary->hash_table_[1]);
	// Set dictionary's data members:
	dictionary->type_ = type;
	dictionary->argument_ = argument;
	dictionary->rehash_index_ = -1; // Not in rehashing.
	dictionary->iterator_number_ = 0;
	return dictionary;
}

// Perform rehash_count steps of incremental rehashing. Return 1 if there are still
// keys to move from the old to the new hash table, otherwise 0 is returned.
// A rehashing step consists in moving a slot, which may have more than one key
// as we use chaining, from the old to the new hash table.
// Since part of the hash table may be composed of empty slots, so we allow visiting
// empty slot for at most rehash_count*10 times, otherwise the amount of work
// it does would be unbound and the function may block for a long time.
// O(N)
int DictionaryRehash(Dictionary *dictionary, int rehash_count)
{
	// 1. Check whether in the process of rehashing. If not, return.
	if(DictionaryIsRehashing(dictionary) == 0)
	{
		return 0;
	}

	// 2. Try to perform rehash_count steps of rehashing.
	HashTable *first_hash_table = &(dictionary->hash_table_[0]),
	           *second_hash_table = &(dictionary->hash_table_[1]);
	int empty_visit = rehash_count * 10; // Max number of empty slots to visit.
	while(rehash_count-- && first_hash_table->element_number_ != 0)
	{
		// Make sure rehash_index_ can't overflow.
		assert((unsigned)dictionary->rehash_index_ < first_hash_table->size_);
		// Empty slots don't need rehash.
		while(first_hash_table->slot_[dictionary->rehash_index_] == NULL)
		{
			++dictionary->rehash_index_;
			if(--empty_visit == 0)
			{
				return 1; // Only allow visit at most 10*rehash_count empty slots.
			}
		}

		// Rehash all the keys in this slot to the second hash table.
		HashTableNode *current_node = first_hash_table->slot_[dictionary->rehash_index_];
		HashTableNode *next_node = NULL;
		unsigned int new_slot_index = 0;
		while(current_node != NULL)
		{
			next_node = current_node->next_; // Store next hash table node.
			new_slot_index = DictionaryHashKey(dictionary, current_node->key)
			                 & second_hash_table->size_mask_;

			// Add this node to the head of corresponding slot's head.
			current_node->next_ = second_hash_table->slot_[dictionary->rehash_index_];
			second_hash_table->slot_[dictionary->rehash_index_] = current_node;

			// Update two hash tables' elements number.
			--first_hash_table->element_number_;
			++second_hash_table->element_number_;

			// Move to the next node in the same slot.
			current_node = next_node;
		}
		// Mark the rehashed slot in the first hash table as empty(NULL).
		first_hash_table->slot_[dictionary->rehash_index_] = NULL;
		++dictionary->rehash_index_; // Rehash the next slot.
	}

	// 3. Update dictionary's hash table if rehash all done(i.e., the first has no elements).
	if(first_hash_table->element_number_ == 0)
	{
		// Since all elements have rehashed(linked) to the second hash table,
		// free the first hash table's array, whose element is HashTableNode *.
		Free(first_hash_table->slot_);
		DictionaryResetHashTable(first_hash_table); // Omit can okay, supply better.
		// Replace the first hash table by the second: Pass by Value, not by pointer!
		// Otherwise, when we Reset the second hash table, the first is also reseted.
		*first_hash_table = *second_hash_table;
		DictionaryResetHashTable(second_hash_table);
		dictionary->rehash_index_ = -1; // Turn off rehash flag since we finish rehash.
		return 0;
	}
	return 1;
}

// If there are no safe iterators bound to hash table, perform a step of rehashing.
// When we have iterators in the middle of a rehashing, we can't mess with the
// two hash tables, otherwise some element can be missed or duplicated.
// This function is called by common lookup or update operations in the dictionary,
// so that the hash table automatically rehashes while it is actively used.
// O(1)
void DictionaryRehashStep(Dictionary *dictionary)
{
	if(dictionary->iterator_count_ == 0)
	{
		DictionaryRehash(dictionary, 1);
	}
}

// Return the first number that is a power of 2 and is greater than or equal to size.
// O(1)
static unsigned long DictionaryNextPower(unsigned long size)
{
	if(size >= LONG_MAX) // LONG_MAX defined in <limits.h>
	{
		return LONG_MAX;
	}
	unsigned long real_size = DICTIONARY_HASH_TABLE_INITIAL_SIZE; // 4
	for(;;)
	{
		if(real_size >= size)
		{
			return real_size;
		}
		real_size <<= 1; // *2
	}
}

// Create(when dictionary is empty) or Expand the hash table.
// O(1)
int DictionaryExpand(Dictionary *dictionary, unsigned long size)
{
	// real_length is the first number that is a power of 2 and is greater than or equal to length.
	unsigned long real_size = DictionaryNextPower(size);
	// 1. Exclude the conditions that we can't expand the hash table.
	// When at least one of the following condition satisfy, expand is rejected:
	// (1). In the process of rehashing(since we should expand/shrink before rehashing).
	// (2). The specified size is less than the number of elements already in hash table.
	// (3). The real expand size is equal to the current size(legal but useless).
	if(DictionaryIsRehashing(dictionary) || dictionary->element_number_ > size ||
	        real_size == dictionary->hash_table_[0].size_)
	{
		return DICTIONARY_ERROR;
	}
	// 2. Allocate the new hash table and initialize all pointers to NULL.
	HashTable new_hash_table;
	// This hash table has real_size slots.
	new_hash_table.slot_ = Calloc(real_size * sizeof(HashTableNode*));
	new_hash_table.size_ = real_size;
	new_hash_table.size_mask_ = real_size - 1;
	new_hash_table.element_number_ = 0;
	// 3. Choose hash table index of the new_hash_table in the dictionary.
	// (1)	If the first hash table is empty, then this is the first initialization,
	//			set the first hash table so that it can accept keys.
	if(dictionary->hash_table_[0].size_ == 0)
	{
		dictionary->hash_table_[0] = new_hash_table;
	}
	// (2)	Set the second hash table and set rehash index to 0
	//			to indicate the start of incremental rehashing.
	else
	{
		dictionary->hash_table_[1] = new_hash_table;
		dictionary->rehash_index_ = 0;
	}
	return DICTIONARY_SUCCESS;
}

// Expand the hash table if needed.
// O(1)
static int DictionaryExpandIfNeeded(Dictionary *dictionary)
{
	HashTable *hash_table = &(dictionary->hash_table_[0]);
	// 1.	Hash table is empty and expand it to the initial size.
	//		This is to create rather than to expand.
	if(hash_table->size_ == 0)
	{
		return DictionaryExpand(dictionary, DICTIONARY_HASH_TABLE_INITIAL_SIZE);
	}

	// 2. Check whether in process of rehashing. Always expand/shrink before rehashing.
	if(DictionaryIsRehashing(dictionary))
	{
		return DICTIONARY_SUCCESS;
	}

	// 3.	Expand if element_number/slot_number(i.e., size_) >= 1:1, and if
	//			(1) we are allowed to resize the hash table(dictionary_can_resize), or
	//			(2) this ratio is over the safe threshold(dictionary_force_resize_ratio),
	//		we resize doubling the number of buckets.
	int element_slot_ratio = hash_table->element_number_ / hash_table->size_;
	if(element_slot_ratio >= 1 &&
	        (dictionary_can_resize || element_slot_ratio > dictionary_force_resize_ratio))
	{
		return DictionaryExpand(dictionary, hash_table->element_number_ * 2);
	}
	return DICTIONARY_SUCCESS;
}

// Return the index of a slot to which that we can add `key`.
// -1 means the same key already exists.
// When in the process of rehashing, the returned index is always in the second hash
// table, that guarantee the elements_number of the first hash table can't increase.
// O(N)
static int DictionaryKeyIndex(Dictionary *dictionary, const void *key)
{
	// DictionaryExpandIfNeeded() is only used in DictionaryKeyIndex(), and
	// DictionaryKeyIndex() is only used in DictionaryAddRaw(). This fact means
	// that we should check whether our dictionary(hash table) needs expand before
	// we want to add elements.

	// 1. Expand the hash table if needed since we want to add new key-value pair.
	if(DictionaryExpandIfNeeded(dictionary) == DICTINOARY_ERROR)
	{
		return -1;
	}

	// 2. Calculate the hash value of key.
	unsigned int hash_value = DictionaryHashKey(dictionary, key), slot_index = 0;

	// 3.	Calculate the slot index that should added to and check whether the same key
	//		already exists in target hash table.
	// Not in rehashing: add element to the first hash table([0]); otherwise second.
	int index = (DictionaryIsRehashing(dictionary) ? 1 : 0);
	// slot_index = hash_value & added_to_hash_table_size_mask;
	slot_index = hash_value & dictionary->hash_table_[index].size_mask_;
	HashTableNode *node = dictionary->hash_table_[index].slot_[slot_index];
	while(node) // If the target slot is not empty, check if there is the same key.
	{
		if(DictionaryCompareKeys(dictionary, key, node->key))
		{
			// The same key(compared by KeyCompare(), if any, or ==) already exists.
			// Dictionary allows two different keys that have the same hash value
			// (if happens, use chaining), disallows two elements that have the same key.
			return -1;
		}
		node = node->next_;
	}
	return slot_index;
}

// Add a new node to dictionary without setting its value, and return the added
// node's pointer, which let the user fill the value field as he wishes.
// This function is exposed to the user API to be called mainly in order to
// store non-pointers inside the hash value, example:
//    node = DictionaryAddRaw(dictionary, my_key);
//    if (node != NULL) DictionarySetSignedIntegerVal(node, 1000);
// Return: NULL if key already exists; otherwise return the hash node's pointer.
// O(1)
HashTableNode *DictionaryAddRaw(Dictionary *dictionary, void *key)
{
	// 1.	Check whether is in the process of incremental rehashing.
	//		If so, perform a step of incremental rehashing.
	int is_rehashing = DictionaryIsRehashing(dictionary);
	if(is_rehashing)
	{
		DictionaryRehashStep(dictionary);
	}

	// 2.	Check whether key already exists in dictionary.
	//		If so, return NULL; otherwise return the index of hashed slot.
	int slot_index = 0; // slot_index = HashFunction(key) & size_mask_;
	if((slot_index = DictionaryKeyIndex(dictionary, key)) == -1)
	{
		// The same key(compared by KeyCompare(), if any, or ==) already exists
		// in dictionary. Dictionary allows two different keys that have the same hash
		// value(if happens, use chaining), disallows two elements that have the same key.
		return NULL;
	}

	// 3. Select the hash table add to, Construct a new node, Initialize key_ and next_ field.
	// Always add new element to hash_table[1] when incremental rehashing to
	// guarantee that the element_number_ of hash_table[0] don't increase.
	HashTable *hash_table = is_rehashing ?
	                        &dictionary->hash_table_[1] : &dictionary->hash_table_[0];
	HashTableNode *node = Malloc(sizeof(HashTableNode)); // Get new node.
	// Initialize node member: key_, next_. Don't set value_ field, see notes above.
	DictionarySetKey(dictionary, node, key);
	// Add new node to the head of corresponding slot in hash table. O(1)
	node->next_ = hash_table->slot_[slot_index];
	hash_table->slot_[slot_index] = node;

	// 4. Update this hash table's elements' number.
	++hash_table->element_number_;

	return node;
}

// Add a pair of key-value to dictionary.
// O(1)
int DictionaryAdd(Dictionary *dictionary, void *key, void *value)
{
	// 1. Get the added node's pointer by DictionaryAddRaw.
	HashTableNode *node = DictionaryAddRaw(dictionary, key);
	if(node == NULL) // Check availability.
	{
		return DICTIONARY_ERROR;
	}
	// 2. Set node's value as user wishes by DictionarySetValue.
	DictionarySetValue(dictionary, node, value);
	return DICTIONARY_SUCCESS;
}

// Return the pointer to the node whose key matches the argument key.
// NULL if can't find.
HashTableNode *DictionaryFind(Dictionary *dictionary, const void *key)
{
	// 1. Check if dictionary has elements. If not, return NULL directly.
	if(dictionary->hash_table_[0].size_ == 0)
	{
		return NULL;
	}

	// 2. Always try to perform one step of incremental rehash before actual operation.
	if(DictionaryIsRehashing(dictionary))
	{
		DictionaryRehashStep(dictionary);
	}

	// 3. Find the node in dictionary whose key matches the specified key.
	unsigned int hash_value = DictionaryHashKey(dictionary, key);
	for(int index = 0; index <= 1; ++index)
	{
		unsigned int slot_index = hash_value & dictionary->hash_table_[index].size_mask_;
		HashTableNode *node = dictionary->hash_table_[index].slot_[slot_index];
		while(node)
		{
			if(DictionaryCompareKeys(dictionary, key, node->key_))
			{
				return node;
			}
			node = node->next_;
		}
		//(1)	When not in the process of incremental rehashing, only find in the first hash
		//		table since the second hash table is only used in rehashing. If can't find, break
		//		loop and return NULL.
		//(2)	When in rehashing, find in both hash tables since the original node may has
		//		rehashed to the second hash table. If can't find in both hash tables, break loop.
		if(DictionaryIsRehashing(dictionary) == 0)
		{
			break;
		}
	}
	return NULL;
}

// Add a new key-value pair if the key doesn't exist in dictionary yet;
// otherwise replace current value with argument value.
// Return 1 if the new key-value pair is added; 0 if only replace value.
int DictionaryReplace(Dictionary *dictionary, void *key, void *value)
{
	// 1. Try to add the key-value pair into dictionary.
	if(DictionaryAdd(dictionary, key, value) == DICTIONARY_SUCCESS)
	{
		return 1; // Add new key-value pair.
	}

	// 2. Find the already exist key-value pair and Replace the old value.
	// node_ptr is a pointer that points to the actual element(node) in the dictionary,
	// we use it to replace the HashTableNode.union_value_.value_(a void*).
	HashTableNode *node_ptr = DictionaryFind(dictionary, key);
	// node_copy is a HashTableNode that is a copy of the node before replacing value
	// field. Every HashTableNode stores its value as a void*, and if we don't save a copy
	// of original node, its value field pointer would can't be found after replacing, thus
	// we can't free its value, causing memory leak. In summary, node_copy stores
	// the pointer to the previous value, and we use it to free the previous value's memory.
	HashTableNode old_node_copy = *node_ptr;
	// Set the new value and free the old one. We must do that in this order, as the value
	// may be the same as the previous one(Self-Assignment case).
	DictionarySetValue(dictionary, node_ptr, value);
	DictionaryFreeValue(dictionary, &old_node_copy);
	return 0;
}

// Get the value of element whose key matches the supplied key.
void DictionaryGetValue(Dictionary *dictionary, const void *key)
{
	HashTableNode *node = DictionaryFind(dictionary, key);
	return node ? DictionaryGetElementValue(node) : NULL;
}

// Return SUCCESS if delete the node whose key matches the argument key,
// ERROR if can't find.
static int DictionaryGenericDelete(Dictionary *dictionary, const void *key, int no_free)
{
	// 1. Check if dictionary has elements. If not, return NULL directly.
	if(dictionary->hash_table_[0].size_ == 0)
	{
		return NULL;
	}

	// 2. Always try to perform one step of incremental rehash before actual operation.
	if(DictionaryIsRehashing(dictionary))
	{
		DictionaryRehashStep(dictionary);
	}

	// 3. Find the node in dictionary whose key matches the specified key.
	unsigned int hash_value = DictionaryHashKey(dictionary, key);
	for(int index = 0; index <= 1; ++index)
	{
		unsigned int slot_index = hash_value & dictionary->hash_table_[index].size_mask_;
		HashTableNode *previous_node = NULL;
		HashTableNode *node = dictionary->hash_table_[index].slot_[slot_index];
		while(node)
		{
			if(DictionaryCompareKeys(dictionary, key, node->key_))
			{
				if(previous_node)
				{
					previous_node->next_ = node->next_;
				}
				else
				{
					dictionary->hash_table_[index].slot_[slot_index] = node->next_;
				}
				if(no_free == 0)
				{
					DictionaryFreeKey(dictionary, node);
					DictionaryFreeValue(dictionary, node);
				}
				Free(node);
				--dictionary->hash_table_[index].element_number_;
				return DICTIONARY_SUCCESS;
			}
			previous_node = node;
			node = node->next_;
		}
		//(1)	When not in the process of incremental rehashing, only find in the first hash
		//		table since the second hash table is only used in rehashing. If can't find, break
		//		loop and return NULL.
		//(2)	When in rehashing, find in both hash tables since the original node may has
		//		rehashed to the second hash table. If can't find in both hash tables, break loop.
		if(DictionaryIsRehashing(dictionary) == 0)
		{
			break;
		}
	}
	return DICTIONARY_ERROR;
}

int DictionaryDelete(Dictionary *dictionary, const void *key)
{
	return DictionaryGenericDelete(dictionary, key, 0);
}

int DictionaryDeleteNoFree(Dictionary *dictionary, const void *key)
{
	return DictionaryGenericDelete(dictionary, key, 1);
}

// Destroy specified hash table of dictionary.
void DictionaryClear(Dictionary *dictionary, HashTable *hash_table, void (callback)(void*))
{
	HashTableNode *node = NULL, *next_node = NULL;
	for(unsigned long index = 0;
	        index < hash_table->size_ && hash_table->element_number_ > 0; ++index)
	{
		if(callback && (index & 65535) == 0) // TODO: What use?
		{
			callback(dictionary->argument_);
		}
		if((node = hash_table->slot_[index]) == NULL)
		{
			continue;
		}
		while(node)
		{
			next_node = node;
			DictionaryFreeKey(dictionary, node);
			DictionaryFreeValue(dictionary, node);
			Free(node);
			--hash_table->element_number_;
			node = next_node;
		}
	}
	Free(hash_table->slot_);
	DictionaryResetHashTable(hash_table);
}

// Clear and free dictionary's memory.
void DictionaryRelease(Dictionary *dictionary)
{
	DictionaryClear(dictionary, &dictionary->hash_table_[0], NULL);
	DictionaryClear(dictionary, &dictionary->hash_table_[1], NULL);
	Free(dictionary);
}

