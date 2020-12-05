#ifndef GX_UTIL_H
#define GX_UTIL_H

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//... types:: alias
typedef SDL_Rect sRect;
typedef SDL_Point sPoint;
typedef SDL_Point sVector;
typedef Uint32 sElemId;

//... types::containers
typedef struct sList sList;
typedef struct sArray sArray;
typedef struct sMap sMap;
typedef struct sQtree sQtree;
typedef struct sQtreeElem sQtreeElem;

//... types::main structs
typedef struct sApp sApp;
typedef struct sFolder sFolder;
typedef struct sScene sScene;
typedef struct sElement sElement;
typedef struct sIni sIni;

//... types:: auxiliary
typedef struct sGraphics sGraphics;
typedef struct sPhysics sPhysics;
typedef struct sContact sContact;
typedef struct sEvent sEvent;
typedef struct sImage sImage;
typedef struct sAnimation sAnimation;
typedef struct sChunk sChunk;
typedef struct sMusic sMusic;
struct sElemRenderable;
struct sElemBody;

//... functions:: alias
typedef void (*sDtor)();
typedef int (*GxComp)(const void*, const void*);
typedef void (*sHandler)(sEvent*);

//... types:: definitions
typedef struct sSize {
	int w;
	int h;
} sSize;

typedef struct sMatrix {
	int nr;
	int nc;
} sMatrix;

typedef struct sEvent {
	void* target;
	int type;
	union { sContact* contact; SDL_Event* sdle; };
} sEvent;

//... sIni
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
	sRect* position;

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


//... Util namespace
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
	void (*onDestroyFreeTarget)(sEvent* e);
	void(*onDestroyDoNothing)(sEvent* e);
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

	struct sUtilStatusNamespace {		
		const int NONE;
		const int LOADING;
		const int LOADED;
		const int RUNNING;
		const int PAUSED;
		const int READY;
		const int UNLOADING;
	}* status;

	struct sUtilHash {
		const Uint32 ELEMENT;
		const Uint32 SCENE;
		const Uint32 CONTACT;
	}* hash;
}* nUtil;

#endif // !UTILITIES_H
