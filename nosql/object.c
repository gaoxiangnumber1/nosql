#include <nosql.h>

void DecreaseReferenceCount(NosqlObject *object)
{
	if(object->reference_count_ <= 0)
	{
		// TODO: LOG_ERROR.
	}
	else if(object->reference_count_ == 1)
	{
		// TODO: switch(object->type_) {...}
		object->reference_count_ = 0;
		Free(object);
	}
	else
	{
		--object->reference_count_;
	}
}
