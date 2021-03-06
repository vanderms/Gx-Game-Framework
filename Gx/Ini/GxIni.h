#ifndef GX_INI_H
#define GX_INI_H
#include "../Public/GxPublic.h"


//... MAIN STRUCT
typedef struct GxIni {
	
	//app
	const char* title;
	const char* window;
	
	//scene
	GxSize size;
	int gravity;
	const char* folders;

	//tilemap
	int* sequence;
	GxMatrix matrix;
	
	//element
	const char* name;
	const char* className;
	int display;
	int body;
	const SDL_Rect* position;

	//widget
	int zIndex;
	int orientation;
	const char* image;
	const char* animation;
	int animationRepeat;
	const char* alignment;	
	bool hidden;
	double angle;
	double proportion;
	const char* border;
	const char* color;	
	const char* backgroundColor;
	const char* text;
	int fontSize;
	const char* font;
	
	//body	
	GxVector velocity;	
	bool friction;

	//callbacks
	void* target;
	GxHandler onLoad;
	GxHandler onLoopBegin;
	GxHandler onUpdate;
	GxHandler onPreGraphical;
	GxHandler onPreRender;
	GxHandler onLoopEnd;
	GxHandler onUnload;	
	GxHandler onDestroy;
	GxHandler onKeyboard;
	GxHandler onMouse;
	GxHandler onFinger;
	GxHandler onSDLDefault;
	GxHandler onPreContact;
	GxHandler onContactBegin;
	GxHandler onContactEnd;
	GxHandler onElemRemove;
} GxIni;

#endif // !GX_INI_H
