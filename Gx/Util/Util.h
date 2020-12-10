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
typedef int (*sComp)(const void*, const void*);
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


enum sEventType {
	nEvent_ON_LOAD,
	nEvent_ON_LOOP_BEGIN,
	nEvent_ON_UPDATE,
	nEvent_ON_RENDER,
	nEvent_ON_LOOP_END,
	nEvent_ON_UNLOAD,
	nEvent_ON_KEYBOARD,
	nEvent_ON_MOUSE,
	nEvent_ON_FINGER,
	nEvent_ON_SDL_DEFAULT,
	nEvent_ON_PRE_CONTACT,
	nEvent_ON_CONTACT_BEGIN,
	nEvent_ON_CONTACT_END,
	nEvent_ON_TIMEOUT,
	nEvent_ON_DESTROY,
	nEvent_ON_ELEM_REMOVAL,
	nEvent_TOTAL,
};

typedef struct sEvent {
	void* target;
	enum sEventType type;
	union { sContact* contact; SDL_Event* sdle; };
} sEvent;


//...sComponent
typedef struct sComponent {
	void* target;
	const char* name;
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
} sComponent;

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

	//component
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
enum sUtilStatus{
	nUtil_STATUS_NONE,
	nUtil_STATUS_LOADING,
	nUtil_STATUS_LOADED,
	nUtil_STATUS_RUNNING,
	nUtil_STATUS_PAUSED,
	nUtil_STATUS_READY,
	nUtil_STATUS_UNLOADING
};
	
extern const Uint32 nUtil_HASH_ELEMENT;
extern const Uint32 nUtil_HASH_SCENE;
extern const Uint32 nUtil_HASH_CONTACT;
	

int* nUtilCreateInt(int value);
Uint32* nUtilCreateUint(Uint32 value);
bool* nUtilCreateBool(bool value);
double* nUtilCreateDouble(double value);
char* nUtilCreateString(const char* value);
char* nUtilCreateStringF(const char* format, ...);
char* nUtilCloneString(const char* str, char* buffer, unsigned int size);	
sArray* nUtilSplit(const char* str, const char* sep);
char* nUtilTrim(const char* str, char* buffer, size_t bSize);
int nUtilRandom(uint32_t* seed, int start, int end);
void nUtilPrintMask(Uint32 mask);
SDL_Point nUtilCalcDistance(const SDL_Point* pointA, const SDL_Point* pointB);
bool nUtilAssertNullPointer(const void* ptr); 
bool nUtilAssertArgument(bool condition);
bool nUtilAssertState(bool condition);
void* nUtilAssertAlloc(void* ptr);
bool nUtilAssertResourceNotFound(bool condition);
bool nUtilAssertImplementation(bool condition);
bool nUtilAssertHash(bool condition);
bool nUtilAssertOutOfRange(bool condition);	
void nUtilSplitAssetPath(const char* path, char* folder, char* asset);


//...component namespace
		
bool nComponentIsIniEmpty_(const sIni* ini);
bool nComponentIsEmpty_(const sComponent* comp);
sComponent* nComponentCreate_(const sIni* ini);
sComponent* nComponentCopy_(const sComponent* comp);
void nComponentDestroy_(sComponent* self);
sHandler nComponentGetHandler_(sComponent* comp, int type);
void nOnDestroyFreeTarget(sEvent* e);
void nOnDestroyDoNothing(sEvent* e);

#endif // !UTILITIES_H
