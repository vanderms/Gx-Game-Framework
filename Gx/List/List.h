#ifndef GX_LIST_H
#define GX_LIST_H
#include "../Utilities/Util.h"

extern const struct sListNamespace {
	
	//...
	sList* (*create)(void);
	void (*destroy)(sList* self);

	//...
	int (*size)(sList* self);
	void* (*first)(sList* self);
	void* (*last)(sList* self);
	void* (*at)(sList* self, int index);
	bool (*contains)(sList* self, void* value);
	void* (*begin)(sList* self);
	void* (*next)(sList* self);
	void (*push)(sList* self, void* value, sDtor dtor);
	void (*insert)(sList* self, int index, void* value, sDtor dtor);
	bool (*replace)(sList* self, void* oldValue, void* newValue, sDtor dtor);
	bool (*remove)(sList* self, void* value);
	bool (*removeByIndex)(sList* self, int index);
	void (*clean)(sList* self);
}* nList;

#endif // !GX_LIST_H
