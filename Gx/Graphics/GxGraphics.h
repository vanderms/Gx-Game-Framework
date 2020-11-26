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

//...Element
void GxElemRender_(GxElement* self);
SDL_Rect GxGetElemPositionOnWindow(GxElement* self);
 SDL_Rect* GxElemCalcImagePos(GxElement* self, SDL_Rect* pos, GxImage* image);

 //...Assets
 void GxImageRender_(GxImage* self, SDL_Rect* target, 
    double angle, SDL_RendererFlip orientation, Uint8 opacity
);
void GxImageRenderTilePalette_(GxImage* self, SDL_Rect* target, Uint8 opacity);

#endif // !GX_GRAPHICS_H
