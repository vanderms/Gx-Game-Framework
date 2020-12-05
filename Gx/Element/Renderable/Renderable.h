#ifndef GX_RENDERABLE_H
#define GX_RENDERABLE_H
#include "../../Utilities/Util.h"


extern const struct sElemRenderableNamespace {

	bool (*hasRelativePosition)(sElement* self);
	bool (*hasAbsolutePosition)(sElement* self);

	int (*zIndex)(sElement* self);
	void (*setZIndex)(sElement* self, int index);

	Uint8 (*opacity)(sElement* self);
	void (*setOpacity)(sElement* self, Uint8 value);

	int (*orientation)(sElement* self);
	void (*setOrientation)(sElement* self, int value);

	const char* (*image)(sElement* self);
	void (*setImage)(sElement* self, const char* apath);	

	const char* (*animation)(sElement* self);
	void (*setAnimation)(sElement* self, const char* apath);

	const char* (*alignment)(sElement* self);
	void (*setAlignment)(sElement* self, const char* value);

	bool (*isHidden)(sElement* self);
	void (*hide)(sElement* self);
	void (*show)(sElement* self);

	double (*angle)(sElement* self);
	void (*setAngle)(sElement* self, double angle);

	double (*proportion)(sElement* self);
	void (*setProportion)(sElement* self, double proportion);
	void (*setToFit)(sElement* self, const char* axis);

	const SDL_Color* (*backgroundColor)(sElement* self);
	void (*setBackgroundColor)(sElement* self, const char* color);

	int (*borderSize)(sElement* self);
	const SDL_Color*(*borderColor)(sElement* self);
	void (*setBorder)(sElement* self, const char* border);

	const char* (*text)(sElement* self);
	void (*setText)(sElement* self, const char* text, ...);
	
	int (*fontSize)(sElement* self);
	void (*setFontSize)(sElement* self, int size);	

	void (*setFont)(sElement* self, const char* font);
	const char* (*font)(sElement* self);

	const SDL_Color* (*color)(sElement* self);
	void (*setColor)(sElement* self, const char* color);

	sRect (*calcPosOnCamera)(sElement* self);	
	
	struct sElemRenderablePrivateNamespace {
		struct sElemRenderable* (*create)(sElement* elem, const sIni* ini);
		void (*destroy)(struct sElemRenderable* self);
		void (*onRender)(sElement* self);	
		sQtreeElem* (*getQtreeElem)(sElement* self);
		sRect* (*calcImagePosOnCamera)(sElement* self, sRect* pos, sImage* image);
		sImage* (*getImageRef)(sElement* self);
		sImage* (*label)(sElement* self);

	}* p;
} nElemRenderable;


#endif // !GX_RENDERABLE_H
