#ifndef GX_ARRAY_H
#define GX_ARRAY_H
#include "../../Util/Util.h"

sArray* nArrayCreate(void);
void nArrayDestroy(sArray* self);	
Uint32 nArraySize(sArray* self);
Uint32 nArrayCapacity(sArray* self);
void* nArrayAt(sArray* self, Uint32 index);
void* nArrayLast(sArray* self);
void nArrayPush(sArray* self, void* value, sDtor dtor);
void nArrayInsert(sArray* self, Uint32 index, void* value, sDtor dtor);
void nArrayRemove(sArray* self, Uint32 index);
int nArrayRemoveByValue(sArray* self, void* value);
int64_t nArrayIndexOf(sArray* self, void* value);
void nArrayReserve(sArray* self, Uint32 capacity);
void nArrayClean(sArray* self);
void nArraySort(sArray* self, sComp compare);	

#endif // !GX_ARRAY_H



