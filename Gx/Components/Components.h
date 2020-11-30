#ifndef GX_COMPONENTS_H
#define GX_COMPONENTS_H
#include "../Gx.h"

typedef struct ButtonNamespace {
	GxElement* (*create)(const GxIni* ini, Uint32 inputs, int keyCode);	
	Uint32 (*getStatus)(GxElement* elem);
	bool (*hasStatus)(GxElement* elem, Uint32 status);
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
} ButtonNamespace;


typedef struct ComponentNamespace {
	const ButtonNamespace* button;
}ComponentNamespace;

extern const ButtonNamespace ComponentButtonNamespaceInstance;

static const ComponentNamespace* component = &(ComponentNamespace){
	.button = &ComponentButtonNamespaceInstance,
};


#endif // !GX_COMPONENTS_H
