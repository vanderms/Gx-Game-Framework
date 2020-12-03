#ifndef GX_GRAPHICS_H
#define GX_GRAPHICS_H
#include "../Utilities/Util.h"

extern const struct sGraphicsNamespace {
	sGraphics* (*create)(sScene* scene);
	void (*destroy)(sGraphics* self);
	void (*insert)(sGraphics* self, sElement* element);
	void (*updateElement)(sGraphics* self, sElement* element, SDL_Rect previousPos);
	void (*remove)(sGraphics* self, sElement* element);
	void (*update)(sGraphics* self);
	//...sElement
	void (*renderElement)(sElement* self);
}* nGraphics;



#endif // !GX_GRAPHICS_H
