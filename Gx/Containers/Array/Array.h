#ifndef GX_ARRAY_H
#define GX_ARRAY_H
#include "../../Util/Util.h"


extern const struct sArrayNamespace {
	sArray* (*create)(void);
	void (*destroy)(sArray* self);	
	Uint32 (*size)(sArray* self);
	Uint32 (*capacity)(sArray* self);
	void* (*at)(sArray* self, Uint32 index);
	void* (*last)(sArray* self);
	void (*push)(sArray* self, void* value, sDtor dtor);
	void (*insert)(sArray* self, Uint32 index, void* value, sDtor dtor);
	void (*remove)(sArray* self, Uint32 index);
	int (*removeByValue)(sArray* self, void* value);
	int64_t (*indexOf)(sArray* self, void* value);
	void (*reserve)(sArray* self, Uint32 capacity);
	void (*clean)(sArray* self);
	void (*sort)(sArray* self, sComp compare);	
}* const nArray;

#endif // !GX_ARRAY_H



