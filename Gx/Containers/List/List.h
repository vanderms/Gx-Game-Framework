#ifndef GX_LIST_H
#define GX_LIST_H
#include "../../Util/Util.h"


sList* nListCreate(void);
void nListDestroy(sList* self);

int nListSize(sList* self);
void* nListFirst(sList* self);
void* nListLast(sList* self);
void* nListAt(sList* self, int index);
bool nListContains(sList* self, void* value);
void* nListBegin(sList* self);
void* nListNext(sList* self);
void nListPush(sList* self, void* value, sDtor dtor);
void nListInsert(sList* self, int index, void* value, sDtor dtor);
bool nListReplace(sList* self, void* oldValue, void* newValue, sDtor dtor);
bool nListRemove(sList* self, void* value);
bool nListRemoveByIndex(sList* self, int index);
void nListClean(sList* self);


#endif // !GX_LIST_H
