#ifndef NOSQL_SRC_HASH_TABLE_H_
#define NOSQL_SRC_HASH_TABLE_H_

#include <stdint.h>

#ifndef CAST
#define CAST(type) (type)
#endif

#define DICTIONARY_SUCCESS 1
#define DICTIONARY_ERROR 0
#define DICTIONARY_HASH_TABLE_INITIAL_SIZE 4

typedef struct HashTableNode
{
	void *key_; // Key
	union
	{
		void *value_;
		uint64_t uint64_value_;
		int64_t int64_value_;
		double double_value_;
	} union_value_; // Value
	// We use chaining to solve collision(two or more keys hash to the same slot):
	// place all the elements that hash to the same slot into the same linked list.
	// The next_ field is used to link node one by one to make up a link list.
	struct HashTableNode *next_;
} HashTableNode;

typedef struct HashTable
{
	// slot_ is an array of pointers to HashTableNode. Every pointer can be think of
	// as a slot and it points to the first element in its slot.
	HashTableNode **slot_;
	int size_; // The number of slots.
	int size_mask_; // size_mask_ = size_ -1, it is used to calculate slot index.
	// Since we use chaining to solve collision, there may be many nodes chained in
	// the same slot. This field stores the number of all elements in hash table, which is
	// may be greater than size_.
	int element_number_;
} HashTable;

typedef struct HashTableType
{
	int (*HashFunction) (const void *key); // Calculate hash value.
	int (*KeyCompare) (void *argument, const void *key1, const void *key2); // Compare keys.
	void* (*KeyDuplicate) (void *argument, const void *key); // Duplicate key.
	void* (*ValueDuplicate) (void *argument, const void *value); // Duplicate value.
	void (*KeyDestructor) (void *argument, void *key); // Destruct key.
	void (*ValueDestructor) (void *argument, void *value); // Destruct value.
} HashTableType;

typedef struct Dictionary
{
	// Every dictionary has two hash table structures to implement
	// incremental rehashing, for the old to the new table.
	HashTable hash_table_[2];
	HashTableType *type_; // Functions that are type-specific
	void *argument_; // Argument that passed to functions pointed by type_.
	int rehash_index_; // The index that is rehashing in progress. -1: stop; 0: start.
	int iterator_number_; // The number of iterators currently running.
} Dictionary;

// Functions implemented as macros:
#define DictionaryIsRehashing(dictionary) ((dictionary)->rehash_index_ != -1)
// Calculate the hash key's value.
#define DictionaryHashKey(dictionary, key) (dictionary)->type_->HashFunction(key)
// Compare two keys by dictionary's own KeyCompare(), if any, or use ordinary equal.
#define DictionaryCompareKeys(dictionary, key1, key2) \
(((dictionary)->type_->KeyCompare) ? \
	(dictionary)->type_->KeyCompare((dictionary)->argument_, key1, key2) : \
	(key1) == (key2))
// Set the node's key by KeyDuplicate(), if any, or share the argument key.
#define DictionarySetKey(dictionary, node, key) \
node->key_ = (((dictionary)->type_->KeyDuplicate) ? \
	(dictionary)->type_->KeyDuplicate((dictionary)->argument_, key) : \
	(key))
// Set the node's value by ValueDuplicate(), if any, or share the argument value.
#define DictionarySetValue(dictionary, node, value) \
node->union_value_.value_ = (((dictionary)->type_->ValueDuplicate) ? \
	(dictionary)->type_->ValueDuplicate((dictionary)->argument_, value) : \
	(value))
// Free the node's key memory by KeyDestructor().
#define DictionaryFreeKey(dictionary, node) \
if((dictionary)->type_->KeyDestructor) \
	(dictionary)->type_->KeyDestructor((dictionary)->argument_, (node)->key_)
// Free the node's value memory by ValueDestructor().
#define DictionaryFreeValue(dictionary, node) \
if((dictionary)->type_->ValueDestructor) \
	(dictionary)->type_->ValueDestructor((dictionary)->argument_, (node)->union_value_.value_)
// Get specified element's(node's) value. Usually after called DictionaryFind().
#define DictionaryGetElementValue(element) ((element)->union_value_.value_)

// Enable resizing of the hash table.
void DictionaryEnableResize();
// Disable resizing of the hash table.
void DictionaryDisableResize();
// Reset hash table structure's all data members to zero.
void DictionaryResetHashTable(HashTable *hash_table);
// Return a pointer to new dictionary created specified by type and argument.
Dictionary *DictionaryCreate(HashTableType *type, void *argument);
// Perform rehash_count steps of incremental rehashing. Return 1 if there are still
// keys to move from the old to the new hash table, otherwise 0 is returned.
int DictionaryRehash(Dictionary *dictionary, int rehash_count);
// If there are no safe iterators bound to hash table, perform a step of rehashing.
void DictionaryRehashStep(Dictionary *dictionary);
// Create(when dictionary is empty) or Expand the hash table.
int DictionaryExpand(Dictionary *dictionary, const int size);
// Add a new node to dictionary without setting its value, and return the added
// node's pointer, which let the user fill the value field as he wishes.
HashTableNode *DictionaryAddRaw(Dictionary *dictionary, const void *key);
// Add a pair of key-value to dictionary.
int DictionaryAdd(Dictionary *dictionary, const void *key, const void *value);
// Return the pointer to the node whose key matches the argument key.
// NULL if can't find.
HashTableNode *DictionaryFind(Dictionary *dictionary, const void *key);
// Add a new key-value pair if the key doesn't exist in dictionary yet;
// otherwise replace current value with argument value.
// Return 1 if the new key-value pair is added; 0 if only replace value.
int DictionaryReplace(Dictionary *dictionary, const void *key, const void *value);
// Get the value of element whose key matches the supplied key.
void *DictionaryGetValue(Dictionary *dictionary, const void *key);
// Delete specified key-value pair in the dictionary.
int DictionaryDelete(Dictionary *dictionary, const void *key);
// Delete specified key-value pair in the dictionary, not free its key and value.
int DictionaryDeleteNoFree(Dictionary *dictionary, const void *key);
// Destroy specified hash table of dictionary.
void DictionaryClear(Dictionary *dictionary, HashTable *hash_table, void (callback)(void*));
// Clear and free dictionary's memory.
void DictionaryRelease(Dictionary *dictionary);

#endif // NOSQL_SRC_HASH_TABLE_H_
