#ifndef GX_PUBLIC_H
#define GX_PUBLIC_H

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

//... TYPES :: ALIAS
typedef SDL_Rect SDL_Rect;
typedef SDL_Point GxPoint;
typedef SDL_Point GxVector;
typedef Uint32 GxElemID;

//... TYPES :: FORWARD DECLARATION
typedef struct GxList GxList;
typedef struct sArray sArray;
typedef struct GxMap GxMap;
typedef struct GxIni GxIni;
typedef struct GxIEventHandler GxIEventHandler;
typedef struct GxEvent GxEvent;
typedef struct GxRequest GxRequest;
typedef struct GxResponse GxResponse;
typedef struct GxScene GxScene;
typedef struct GxElement GxElement;
typedef struct GxGraphics GxGraphics;
typedef struct GxPhysics GxPhysics;
typedef struct GxContact GxContact;
typedef struct GxRigidBody GxRigidBody;
typedef struct GxRenderable GxRenderable;
typedef struct GxImage GxImage;
typedef struct GxAnimation GxAnimation;
typedef struct GxFolder GxFolder;
typedef struct GxSound GxSound;
typedef struct GxMusic GxMusic;

//... FUNCTIONS :: ALIAS
typedef void (*GxDestructor)();
typedef int (*GxComp)(const void*, const void*);
typedef void (*GxHandler)(GxEvent*);
typedef void (*GxRequestHandler)(GxRequest*, GxResponse*);

//... TYPES :: DEFINITIONS
typedef struct GxSize {
	int w;
	int h;
} GxSize;

typedef struct GxMatrix {
	int nr;
	int nc;
} GxMatrix;

typedef struct GxEvent {
	void* target;
	int type;
	union { GxContact* contact; SDL_Event* sdle; };
} GxEvent;

static inline void GxAssertion_(bool condition, const char* error) {
	if (!condition) {		
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error, NULL);
		SDL_TriggerBreakpoint();
        exit(EXIT_FAILURE);
    }
}

//.. CONSTANTS
#define	GxCmaskNone 0
#define	GxCmaskAll ~0u
#define	GxCmaskCamera 1u << 30
#define	GxCmaskDynamic 1 << 0
#define	GxCmaskFixed (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7)

typedef enum GxContactConstant {
	GxContactRight = 1 << 0,
	GxContactLeft = 1 << 1,
	GxContactHorizontal = 1 << 0 | 1 << 1,
	GxContactUp = 1 << 2,
	GxContactDown = 1 << 3,
	GxContactVertical = 1 << 2 | 1 << 3,
	GxContactPrevented = 1 << 4,
	GxContactAll = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4,
} GxContactConstant;

typedef enum GxStatus {
	GxStatusNone,
	GxStatusLoading,
	GxStatusLoaded,
	GxStatusRunning,
	GxStatusPaused,
	GxStatusReady,
	GxStatusUnloading,
} GxStatus;


//... AUXILIARY TYPES
typedef enum GxEventType {
	//lifecycle
	GxEventOnLoad,
	GxEventOnLoopBegin,
	GxEventOnUpdate,
	GxEventOnPreGraphical,
	GxEventOnPreRender,
	GxEventOnLoopEnd,
	GxEventOnUnload,

	//sdl events
	GxEventOnKeyboard,
	GxEventMouse,
	GxEventFinger,
	GxEventSDLDefault,

	//contact events
	GxEventPreContact,
	GxEventContactBegin,
	GxEventContactEnd,

	//events not inside ihandlers
	GxEventTimeout,
	GxEventOnDestroy,
	GxEventOnElemRemoval,

	//number of events
	GxEventTotalHandlers,
} GxEventType;


enum GxElemConstants {
	GxElemNone = 1,
	GxElemAbsolute = 2,
	GxElemRelative = 3,	
	GxElemFixed = 4,
	GxElemDynamic = 5,	
	GxElemForward = SDL_FLIP_NONE,
	GxElemBackward = SDL_FLIP_HORIZONTAL,
};

#endif // !GX_PUBLIC_H
