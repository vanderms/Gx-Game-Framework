#ifndef GX_QUADTREE_H
#define GX_QUADTREE_H
#include "../Utilities/GxUtil.h"

typedef struct GxQtree GxQtree;

GxQtree* GxCreateQtree_(GxQtree* parent, SDL_Rect pos, const char* type);
void GxDestroyQtree_(GxQtree* self);
SDL_Rect GxQtreeGetPosition_(GxQtree* self);
void GxQtreeInsert_(GxQtree* self, GxElement* element);
void GxQtreeRemove_(GxQtree* self, GxElement* element);
void GxQtreeUpdate_(GxQtree* self, GxElement* element, SDL_Rect previous);
void GxQtreeSubdivide_(GxQtree* self);
void GxQtreeIterate_(GxQtree* self, SDL_Rect area, void(*callback)(GxElement*), bool begin);

#endif // !GX_QUADTREE_H



