#ifndef GX_GRAPHICS_H
#define GX_GRAPHICS_H
#include "../Util/Util.h"


sGraphics* nGraphicsCreate_(sScene* scene);
void nGraphicsDestroy_(sGraphics* self);
void nGraphicsInsert_(sGraphics* self, sElement* element);
void nGraphicsUpdateElement_(sGraphics* self, sElement* element, sRect previousPos);
void nGraphicsRemoveElement_(sGraphics* self, sElement* element);
void nGraphicsUpdate_(sGraphics* self);
//...sElement
void nGraphicsRenderElement_(sElement* self);




#endif // !GX_GRAPHICS_H
