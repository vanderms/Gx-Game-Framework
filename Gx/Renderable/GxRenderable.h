#ifndef GX_WIDGET_H
#define GX_WIDGET_H
#include "../Utilities/Util.h"
#include "../Elem/Element.h"


int GxElemGetZIndex(sElement* self);
void GxElemSetZIndex(sElement* self, int index);

Uint8 GxElemGetOpacity(sElement* self);
void GxElemSetOpacity(sElement* self, Uint8 value);

int GxElemGetOrientation(sElement* self);
void GxElemSetOrientation(sElement* self, int value);

const char* GxElemGetImage(sElement* self);
void GxElemSetImage(sElement* self, const char* apath);
void GxTilemapSetImage_(sElement* self, GxImage* pallete);

const char* GxElemGetAnimation(sElement* self);
void GxElemSetAnimation(sElement* self, const char* apath);

const char* GxElemGetAlignment(sElement* self);
void GxElemSetAlignment(sElement* self, const char* value);

bool GxElemIsHidden(sElement* self);
void GxElemHide(sElement* self);
void GxElemShow(sElement* self);

double GxElemGetAngle(sElement* self);
void GxElemSetAngle(sElement* self, double angle);

double GxElemGetProportion(sElement* self);
void GxElemSetProportion(sElement* self, double proportion);
void GxElemSetToFit(sElement* self, const char* axis);

const SDL_Color* GxElemGetBackgroundColor(sElement* self);
void GxElemSetBackgroundColor(sElement* self, const char* color);

int GxElemGetBorderSize(sElement* self);
void GxElemSetBorder(sElement* self, const char* border);
const SDL_Color* GxElemGetBorderColor(sElement* self);

uint32_t GxElemGetWFlag_(sElement* self);
void GxElemSetWFlag_(sElement* self, uint32_t value);

void GxElemSetText(sElement* self, const char* text, ...);
const char* GxElemGetText(sElement* self);

void GxElemSetFontSize(sElement* self, int size);
int GxElemGetFontSize(sElement* self);

void GxElemSetFont(sElement* self, const char* font);
const char* GxElemGetFont(sElement* self);

const SDL_Color* GxElemGetColor(sElement* self);
void GxElemSetColor(sElement* self, const char* color);

void GxElementUpdateLabel_(sElemRenderable* renderable);

#endif // !GX_WIDGET_H

