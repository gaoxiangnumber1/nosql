#ifndef NOSQL_SRC_HASH_TABLE_H_
#define NOSQL_SRC_HASH_TABLE_H_

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
	unsigned long size_; // The number of slots.
	unsigned long size_mask_; // size_mask_ = size_ -1, it is used to calculate slot index.
	// Since we use chaining to solve collision, there may be many nodes chained in
	// the same slot. This field stores the number of all elements in hash table, which is
	// may be greater than size_.
	unsigned long element_number_;
} HashTable;

typedef struct HashTableType
{
	unsigned int (*HashFunction) (const void *key); // Calculate hash value.
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
}

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
if ((dictionary)->type_->KeyDestructor) \
	(dictionary)->type_->KeyDestructor((dictionary)->argument_, (node)->key_)
// Free the node's value memory by ValueDestructor().
#define DictionaryFreeValue(dictionary, node) \
if ((dictionary)->type_->ValueDestructor) \
	(dictionary)->type_->ValueDestructor((dictionary)->argument_, (node)->union_value_.value_)
// Get specified element's(node's) value. Usually after called DictionaryFind().
#define DictionaryGetElementValue(element) ((element)->union_value_.value_)

#endif // NOSQL_SRC_HASH_TABLE_H_
