#ifndef GX_ARRAY_H
#define GX_ARRAY_H
#include "../Utilities/GxUtil.h"


//constructor and destructor
GxArray* GxCreateArray(void);
void GxDestroyArray(GxArray* self);

//methods
Uint32 GxArraySize(GxArray* self);
Uint32 GxArrayCapacity(GxArray* self);
void* GxArrayAt(GxArray* self, Uint32 index);
void* GxArrayLast(GxArray* self);
void GxArrayPush(GxArray* self, void* value, GxDestructor dtor);
void GxArrayInsert(GxArray* self, Uint32 index, void* value, GxDestructor dtor);
void GxArrayRemove(GxArray* self, Uint32 index);
int GxArrayRemoveByValue(GxArray* self, void* value);
int64_t GxArrayIndexOf(GxArray* self, void* value);
void GxArrayReserve(GxArray* self, Uint32 capacity);
void GxArrayClean(GxArray* self);
void GxArraySort(GxArray* self, GxComp compare);

#endif // !GX_ARRAY_H



