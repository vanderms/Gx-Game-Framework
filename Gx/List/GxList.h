#ifndef GX_LIST_H
#define GX_LIST_H
#include "../Utilities/Util.h"

//struct declaration
typedef struct GxList GxList;

//constructor and destructor
GxList* GxCreateList(void);
void GxDestroyList(GxList* self);

//methods
int GxListSize(GxList* self);
void* GxListFirst(GxList* self);
void* GxListLast(GxList* self);
void* GxListAt(GxList* self, int index);
bool GxListContains(GxList* self, void* value);
void* GxListBegin(GxList* self);
void* GxListNext(GxList* self);
void GxListPush(GxList* self, void* value, GxDestructor dtor);
void GxListInsert(GxList* self, int index, void* value, GxDestructor dtor);
bool GxListReplace(GxList* self, void* oldValue, void* newValue, GxDestructor dtor);
bool GxListRemove(GxList* self, void* value);
bool GxListRemoveByIndex(GxList* self, int index);
void GxListClean(GxList* self);

#endif // !GX_LIST_H
