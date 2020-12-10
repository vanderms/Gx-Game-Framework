#ifndef GX_QUADTREE_H
#define GX_QUADTREE_H
#include "../../Util/Util.h"



sQtreeElem* nQtreeCreateElem(void* elem, const sRect* (*posGetter)(void* elem));
void nQtreeDestroyElem(sQtreeElem* self);
void* nQtreeGetElem(sQtreeElem* self);
sQtree* nQtreeCreate(sQtree* parent, sRect pos);
void nQtreeDestroy(sQtree* self);
sRect nQtreePosition(sQtree* self);
void nQtreeInsert(sQtree* self, sQtreeElem* element);
void nQtreeRemove(sQtree* self, sQtreeElem* element);
void nQtreeUpdate(sQtree* self, sQtreeElem* element, sRect previous);	
void nQtreeGetElementsInArea(sQtree* self, const sRect* area, sArray* arr, bool begin);	

#endif // !GX_QUADTREE_H
