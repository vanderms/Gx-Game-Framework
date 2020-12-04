#ifndef GX_MAP_H
#define GX_MAP_H
#include "../Utilities/Util.h"
#include "../Array/Array.h"


extern const struct sMapNamespace {
	//...
	sMap* (*create)(void);
	void (*destroy)(sMap* self);

	//...
	Uint32 (*size)(sMap* self);
	Uint32 (*capacity)(sMap* self);
	void* (*get)(sMap* self, const char* key);
	void* (*at)(sMap* self, Uint32 index);
	void (*set)(sMap* self, const char* key, void* value, sDtor dtor);
	void (*rehash)(sMap* self, Uint32 capacity);
	void (*remove)(sMap* self, const char* key);
	void (*removeByIndex)(sMap* self, Uint32 index);
	void (*clean)(sMap* self);
}* nMap;


#endif // !GX_MAP_H
