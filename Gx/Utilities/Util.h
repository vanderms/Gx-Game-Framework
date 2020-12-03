#ifndef GX_UTIL_H
#define GX_UTIL_H
#include "../Public/GxPublic.h"


//... INI
typedef struct sIni {
	
	//app
	const char* title;
	const char* window;
	
	//scene
	sSize size;
	int gravity;
	const char* folders;	
	
	//element
	const char* name;
	const char* className;
	int display;
	int body;
	SDL_Rect* position;

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
	sVector velocity;	
	bool friction;

	//callbacks
	void* target;
	sHandler onLoad;
	sHandler onLoopBegin;
	sHandler onUpdate;
	sHandler onRender;	
	sHandler onLoopEnd;
	sHandler onUnload;	
	sHandler onDestroy;
	sHandler onKeyboard;
	sHandler onMouse;
	sHandler onFinger;
	sHandler onSDLDefault;
	sHandler onPreContact;
	sHandler onContactBegin;
	sHandler onContactEnd;
	sHandler onElemRemove;
} sIni;


//... NAMESPACE
extern const struct sUtilNamespace {	
	int* (*createInt)(int value);
	Uint32* (*createUint)(Uint32 value);
	bool* (*createBool)(bool value);
	double* (*createDouble)(double value);
	char* (*createString)(const char* value);
	char* (*createStringF)(const char* format, ...);
	char* (*cloneString)(const char* str, char* buffer, unsigned int size);	
	sArray* (*split)(const char* str, const char* sep);
	char* (*trim)(const char* str, char* buffer, size_t bSize);
	int (*abs)(int value);
	int (*random)(uint32_t* seed, int start, int end);
	void (*printMask)(Uint32 mask);
	SDL_Point (*calcDistance)(const SDL_Point* pointA, const SDL_Point* pointB);
	bool (*assertNullPointer)(const void* ptr); 
	bool (*assertArgument)(bool condition);
	bool (*assertState)(bool condition);
	void* (*assertAlloc)(void* ptr);
	bool (*assertResourceNotFound)(bool condition);
	bool (*assertImplementation)(bool condition);
	bool (*assertHash)(bool condition);
	bool (*assertOutOfRange)(bool condition);
	void (*onDestroyFreeTarget)(GxEvent* e);
	void(*onDestroyDoNothing)(GxEvent* e);
	void (*splitAssetPath)(const char* path, char* folder, char* asset);

	struct sUtilEventNamespace {
		void (*setHandlers)(sHandler* ihandlers, const sIni* ini);
		bool (*hasHandler)(const sIni* ini);
		const int ON_LOAD;
		const int ON_LOOP_BEGIN;
		const int ON_UPDATE;
		const int ON_RENDER;
		const int ON_LOOP_END;
		const int ON_UNLOAD;	
		const int ON_KEYBOARD;
		const int ON_MOUSE;
		const int ON_FINGER;
		const int ON_SDL_DEFAULT;
		const int ON_PRE_CONTACT;
		const int ON_CONTACT_BEGIN;
		const int ON_CONTACT_END;
		const int ON_TIMEOUT;
		const int ON_DESTROY;
		const int ON_ELEM_REMOVAL;
		const int TOTAL;
	}* evn;

	struct sUtilHash {
		const Uint32 ELEMENT;
		const Uint32 SCENE;
		const Uint32 CONTACT;
	}* hash;
}* nUtil;




#endif // !UTILITIES_H
