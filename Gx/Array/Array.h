#ifndef GX_ARRAY_H
#define GX_ARRAY_H
#include "../Utilities/Util.h"


typedef struct sArrayNamespace {
	sArray* (*create)(void);
	void (*destroy)(sArray* self);	
	Uint32 (*size)(sArray* self);
	Uint32 (*capacity)(sArray* self);
	void* (*at)(sArray* self, Uint32 index);
	void* (*last)(sArray* self);
	void (*push)(sArray* self, void* value, GxDestructor dtor);
	void (*insert)(sArray* self, Uint32 index, void* value, GxDestructor dtor);
	void (*remove)(sArray* self, Uint32 index);
	int (*removeByValue)(sArray* self, void* value);
	int64_t (*indexOf)(sArray* self, void* value);
	void (*reserve)(sArray* self, Uint32 capacity);
	void (*clean)(sArray* self);
	void (*sort)(sArray* self, GxComp compare);
} sArrayNamespace;

extern const sArrayNamespace* nArr;


#endif // !GX_ARRAY_H



