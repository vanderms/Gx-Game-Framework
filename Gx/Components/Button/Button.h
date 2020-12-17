#ifndef GX_BUTTON_H
#define GX_BUTTON_H
#include "../../Gx.h"


extern const Uint32 nBUTTON_KEYBOARD;
extern const Uint32 nBUTTON_FINGER;
extern const Uint32 nBUTTON_MOUSE;
extern const Uint32 nBUTTON_SCREEN;
extern const Uint32 nBUTTON_NONE;
extern const Uint32 nBUTTON_ON;	
extern const Uint32 nBUTTON_HOVER;
extern const Uint32 nBUTTON_CLICK;
extern const Uint32 nBUTTON_DOWN;
extern const Uint32 nBUTTON_UP;		

sElement* nButtonCreate(sElement* base, Uint32 inputs, int keycode);
Uint32 nButtonGetStatus(sElement* base);
bool nButtonHasStatus(sElement*base, Uint32 status);



#endif // !GX_BUTTON_H
