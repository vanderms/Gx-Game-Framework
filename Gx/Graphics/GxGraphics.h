#ifndef GX_GRAPHICS_H
#define GX_GRAPHICS_H
#include "../Utilities/GxUtil.h"

typedef struct GxGraphics GxGraphics;

GxGraphics* GxCreateGraphics_(GxScene* scene);
void GxDestroyGraphics_(GxGraphics* self);
void GxGraphicsInsertElement_(GxGraphics* self, GxElement* element);
void GxGraphicsUpdatePosition_(GxGraphics* self, GxElement* element, SDL_Rect previousPos);
void GxGraphicsRemoveElement_(GxGraphics* self, GxElement* element);
void GxGraphicsUpdate_(GxGraphics* self);

#endif // !GX_GRAPHICS_H
