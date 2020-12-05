#ifndef GX_QUADTREE_H
#define GX_QUADTREE_H
#include "../Utilities/Util.h"


extern const struct sQtreeNamespace {
	sQtree* (*create)(sQtree* parent, sRect pos, int type);
	void (*destroy)(sQtree* self);
	sRect (*position)(sQtree* self);
	void (*insert)(sQtree* self, sElement* element);
	void (*remove)(sQtree* self, sElement* element);
	void (*update)(sQtree* self, sElement* element, sRect previous);	
	void (*getAllElementsInArea)(sQtree* self, sRect area, sArray* arr, bool begin);
	const int GRAPHICAL;
	const int FIXED;
	const int DYNAMIC;
}* nQtree;

#endif // !GX_QUADTREE_H
