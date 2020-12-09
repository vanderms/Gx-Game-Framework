#ifndef GX_QUADTREE_H
#define GX_QUADTREE_H
#include "../../Util/Util.h"


extern const struct sQtreeNamespace {
	sQtreeElem* (*createQtreeElem)(void* elem, const sRect* (*posGetter)(void* elem));
	void (*destroyQtreeElem)(sQtreeElem* self);
	void* (*getElem)(sQtreeElem* self);
	sQtree* (*create)(sQtree* parent, sRect pos);
	void (*destroy)(sQtree* self);
	sRect (*position)(sQtree* self);
	void (*insert)(sQtree* self, sQtreeElem* element);
	void (*remove)(sQtree* self, sQtreeElem* element);
	void (*update)(sQtree* self, sQtreeElem* element, sRect previous);	
	void (*getAllElementsInArea)(sQtree* self, const sRect* area, sArray* arr, bool begin);	
}* nQtree;

#endif // !GX_QUADTREE_H
