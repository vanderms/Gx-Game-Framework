#ifndef GX_BUTTON_H
#define GX_BUTTON_H
#include "../../Gx.h"
	

extern const struct sButtonNamespace {
	void (*implement)(sElement* base, Uint32 inputs, int keycode);
	Uint32 (*getStatus)(sElement* base);
	bool (*hasStatus)(sElement*base, Uint32 status);
	const Uint32 KEYBOARD;
	const Uint32 FINGER;
	const Uint32 MOUSE;
	const Uint32 SCREEN;
	const Uint32 NONE;
	const Uint32 ON;	
	const Uint32 HOVER;
	const Uint32 CLICK;
	const Uint32 DOWN;
	const Uint32 UP;	
}* const nButton;

#endif // !GX_BUTTON_H
