#ifndef GX_RENDERABLE_H
#define GX_RENDERABLE_H
#include "../../Util/Util.h"


enum sElemRenderableConstants {
	nElem_DISPLAY_NONE = 1,
	nElem_DISPLAY_ABSOLUTE = 2,
	nElem_DISPLAY_RELATIVE = 3,
	nElem_FORWARD = SDL_FLIP_NONE,
	nElem_BACKWARD = SDL_FLIP_HORIZONTAL,
};

bool nElemHasRelativeDisplay(sElement* self);
bool nElemHasAbsoluteDisplay(sElement* self);

int nElemZIndex(sElement* self);
void nElemSetZIndex(sElement* self, int index);

Uint8 nElemOpacity(sElement* self);
void nElemSetOpacity(sElement* self, Uint8 value);

int nElemOrientation(sElement* self);
void nElemSetOrientation(sElement* self, int value);

const char* nElemImage(sElement* self);
void nElemSetImage(sElement* self, const char* apath);	

const char* nElemAnimation(sElement* self);
void nElemSetAnimation(sElement* self, const char* apath);

const char* nElemAlignment(sElement* self);
void nElemSetAlignment(sElement* self, const char* value);

bool nElemIsHidden(sElement* self);
void nElemHide(sElement* self);
void nElemShow(sElement* self);

double nElemAngle(sElement* self);
void nElemSetAngle(sElement* self, double angle);

double nElemProportion(sElement* self);
void nElemSetProportion(sElement* self, double proportion);
void nElemSetToFit(sElement* self, const char* axis);

const SDL_Color* nElemBackgroundColor(sElement* self);
void nElemSetBackgroundColor(sElement* self, const char* color);

int nElemBorderSize(sElement* self);
const SDL_Color* nElemBorderColor(sElement* self);
void nElemSetBorder(sElement* self, const char* border);

const char* nElemText(sElement* self);
void nElemSetText(sElement* self, const char* text, ...);
	
int nElemFontSize(sElement* self);
void nElemSetFontSize(sElement* self, int size);	

void nElemSetFont(sElement* self, const char* font);
const char* nElemFont(sElement* self);

const SDL_Color* nElemColor(sElement* self);
void nElemSetColor(sElement* self, const char* color);

sRect nElemCalcPosOnCamera(sElement* self);	

sRect* nElemCalcImagePosOnCamera(sElement* self, sRect* pos, sImage* image);

struct sElemRenderable* nElemCreateRenderable_(sElement* elem, const sIni* ini);
void nElemDestroyRenderable_(struct sElemRenderable* self);
void nElemOnRender_(sElement* self);	
sQtreeElem* nElemGetRenderableQtreeElem_(sElement* self);		
sImage* nElemGetImageRef_(sElement* self);
sImage* nElemLabel_(sElement* self);


#endif // !GX_RENDERABLE_H
