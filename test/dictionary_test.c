#include <dictionary.h>

#include <stdio.h>
#include <assert.h>

#include <memory.h>

int HashInt(const void *key)
{
	return *(CAST(int*)key);
}
int CompareInt(void *not_used, const void *number1, const void *number2)
{
	return *(CAST(int*)number1) == *(CAST(int*)number2);
}
void *DuplicateInt(void *not_used, const void *number)
{
	int *ret = Malloc(CAST(int)sizeof(int));
	*ret = *(CAST(int*)number);
	return ret;
}
void DestructorInt(void *not_used, void *number)
{
	Free(number);
}

int main()
{
	HashTableType *type = Malloc(CAST(int)sizeof(HashTableType));
	type->HashFunction = HashInt;
	type->KeyCompare = CompareInt;
	type->KeyDuplicate = DuplicateInt;
	type->ValueDuplicate = DuplicateInt;
	type->KeyDestructor = DestructorInt;
	type->ValueDestructor = DestructorInt;

	Dictionary *d = DictionaryCreate(type, NULL);
	assert(d->hash_table_[0].slot_ == NULL && d->rehash_index_ == -1);
	int arr = 0;
	DictionaryAdd(d, &arr, &arr);
	assert(d->hash_table_[0].size_ == 4 && d->hash_table_[0].element_number_ == 1);

	printf("All passed! Come on!\n");
}
