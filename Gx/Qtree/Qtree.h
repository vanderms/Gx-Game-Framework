#ifndef GX_QUADTREE_H
#define GX_QUADTREE_H
#include "../Utilities/Util.h"

extern const struct sQtreeNamespace {
	sQtree* (*create)(sQtree* parent, sRect pos, const char* type);
	void (*destroy)(sQtree* self);
	sRect (*position)(sQtree* self);
	void (*insert)(sQtree* self, sElement* element);
	void (*remove)(sQtree* self, sElement* element);
	void (*update)(sQtree* self, sElement* element, sRect previous);	
	void (*iterate)(sQtree* self, sRect area, void(*callback)(sElement*), bool begin);
}* nQtree;

#endif // !GX_QUADTREE_H
