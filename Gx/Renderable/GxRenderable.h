#ifndef GX_WIDGET_H
#define GX_WIDGET_H
#include "../Utilities/GxUtil.h"
#include "../Element/GxElement.h"


int GxElemGetZIndex(GxElement* self);
void GxElemSetZIndex(GxElement* self, int index);

int GxElemGetOrientation(GxElement* self);
void GxElemSetOrientation(GxElement* self, int value);

const char* GxElemGetImage(GxElement* self);
void GxElemSetImage(GxElement* self, const char* apath);
void GxTilemapSetImage_(GxElement* self, GxImage* pallete);

const char* GxElemGetAnimation(GxElement* self);
void GxElemSetAnimation(GxElement* self, const char* apath);

const char* GxElemGetAlignment(GxElement* self);
void GxElemSetAlignment(GxElement* self, const char* value);

bool GxElemIsHidden(GxElement* self);
void GxElemHide(GxElement* self);
void GxElemShow(GxElement* self);

double GxElemGetAngle(GxElement* self);
void GxElemSetAngle(GxElement* self, double angle);

double GxElemGetProportion(GxElement* self);
void GxElemSetProportion(GxElement* self, double proportion);
void GxElemSetToFit(GxElement* self, const char* axis);

const SDL_Color* GxElemGetBackgroundColor(GxElement* self);
void GxElemSetBackgroundColor(GxElement* self, const char* color);

int GxElemGetBorderSize(GxElement* self);
void GxElemSetBorder(GxElement* self, const char* border);
const SDL_Color* GxElemGetBorderColor(GxElement* self);

uint32_t GxElemGetWFlag_(GxElement* self);
void GxElemSetWFlag_(GxElement* self, uint32_t value);

void GxElemRender_(GxElement* self);
SDL_Rect GxGetElemPositionOnWindow(GxElement* self);
 SDL_Rect* GxElemCalcImagePos(GxElement* self, SDL_Rect* pos, GxImage* image);

void GxElemSetText(GxElement* self, const char* text, ...);
const char* GxElemGetText(GxElement* self);

void GxElemSetFontSize(GxElement* self, int size);
int GxElemGetFontSize(GxElement* self);

void GxElemSetFont(GxElement* self, const char* font);
const char* GxElemGetFont(GxElement* self);

const SDL_Color* GxElemGetColor(GxElement* self);
void GxElemSetColor(GxElement* self, const char* color);

#endif // !GX_WIDGET_H

