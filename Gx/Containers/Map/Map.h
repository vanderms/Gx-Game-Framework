#ifndef GX_MAP_H
#define GX_MAP_H
#include "../../Util/Util.h"

//...
sMap* nMapCreate(void);
void nMapDestroy(sMap* self);

//...
Uint32 nMapSize(sMap* self);
Uint32 nMapCapacity(sMap* self);
void* nMapGet(sMap* self, const char* key);
void* nMapAt(sMap* self, Uint32 index);
void nMapSet(sMap* self, const char* key, void* value, sDtor dtor);
void nMapRehash(sMap* self, Uint32 capacity);
void nMapRemove(sMap* self, const char* key);
void nMapRemoveByIndex(sMap* self, Uint32 index);
void nMapClean(sMap* self);

#endif // !GX_MAP_H
