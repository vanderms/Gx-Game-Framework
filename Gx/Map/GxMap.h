#ifndef GX_MAP_H
#define GX_MAP_H
#include "../Utilities/Util.h"
#include "../Array/Array.h"

//struct declaration
typedef struct GxMap GxMap;

//constructor and destructor
GxMap* GmCreateMap(void);
void GxDestroyMap(GxMap* self);

//accessors
Uint32 GxMapSize(GxMap* self);
Uint32 GxMapCapacity(GxMap* self);

void* GxMapGet(GxMap* self, const char* key);
void* GxMapAt(GxMap* self, Uint32 index);
void GxMapSet(GxMap* self, const char* key, void* value, GxDestructor dtor);
void GxMapRehash(GxMap* self, Uint32 capacity);
void GxMapRemove(GxMap* self, const char* key);
void GxMapRemoveByIndex(GxMap* self, Uint32 index);
void GxMapClean(GxMap* self);

#endif // !GX_MAP_H
