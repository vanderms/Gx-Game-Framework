#ifndef GX_GRAPHICS_H
#define GX_GRAPHICS_H
#include "../Utilities/Util.h"

typedef struct GxGraphics GxGraphics;

GxGraphics* GxCreateGraphics_(GxScene* scene);
void GxDestroyGraphics_(GxGraphics* self);
void GxGraphicsInsertElement_(GxGraphics* self, sElement* element);
void GxGraphicsUpdatePosition_(GxGraphics* self, sElement* element, SDL_Rect previousPos);
void GxGraphicsRemoveElement_(GxGraphics* self, sElement* element);
void GxGraphicsUpdate_(GxGraphics* self);

//...sElement
void GxElemRender_(sElement* self);
SDL_Rect GxGetElemPositionOnWindow(sElement* self);
 SDL_Rect* GxElemCalcImagePos(sElement* self, SDL_Rect* pos, GxImage* image);

 //...Assets
 void GxImageRender_(GxImage* self, SDL_Rect* target, 
    double angle, SDL_RendererFlip orientation, Uint8 opacity
);
void GxImageRenderTilePalette_(GxImage* self, SDL_Rect* target, Uint8 opacity);

#endif // !GX_GRAPHICS_H
