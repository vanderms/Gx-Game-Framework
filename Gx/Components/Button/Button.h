#ifndef GX_BUTTON_H
#define GX_BUTTON_H
#include "../../Gx.h"


extern const Uint32 nButton_KEYBOARD;
extern const Uint32 nButton_FINGER;
extern const Uint32 nButton_MOUSE;
extern const Uint32 nButton_SCREEN;
extern const Uint32 nButton_NONE;
extern const Uint32 nButton_ON;	
extern const Uint32 nButton_HOVER;
extern const Uint32 nButton_CLICK;
extern const Uint32 nButton_DOWN;
extern const Uint32 nButton_UP;		

void nButtonCreate(sElement* base, Uint32 inputs, int keycode);
Uint32 nButtonGetStatus(sElement* base);
bool nButtonHasStatus(sElement*base, Uint32 status);



#endif // !GX_BUTTON_H
