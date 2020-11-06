#ifndef GX_PUBLIC_H
#define GX_PUBLIC_H

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

//... TYPES :: FORWARD DECLARATION
typedef struct GxList GxList;
typedef struct GxArray GxArray;
typedef struct GxMap GxMap;
typedef struct GxIni GxIni;
typedef struct GxIEventHandler GxIEventHandler;
typedef struct GxEvent GxEvent;
typedef struct GxRequest GxRequest;
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
typedef void* (*GxRequestHandler)(GxRequest*);

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

typedef struct GxRequest {
	void* target;
	const char* request;
	void* data;
} GxRequest;

//...ASSERTIONS
#define GxAssertNullPointer(pointer) GxAssertion_((pointer), "Null Pointer", __func__)
#define GxAssertInvalidArgument(condition) GxAssertion_((condition), "Invalid argument", __func__)
#define GxAssertAllocationFailure(pointer) GxAssertion_((pointer), "Failed to allocate memory", __func__)
#define GxAssertNotImplemented(condition) GxAssertion_((condition), "Trying to access not implemented module", __func__)
#define GxAssertInvalidHash(condition) GxAssertion_((condition), "Invalid hash", __func__)
#define GxAssertOutOfRange(condition) GxAssertion_((condition), "Out of range", __func__)
#define GxAssertInvalidOperation(condition) GxAssertion_((condition), "Invalid operation", __func__)
#define GxAssertNotFoundAsset(condition) GxAssertion_((condition), "Not found asset", __func__);

static inline void GxAssertion_(bool condition, const char* message, const char* func) {
	if (!condition) {
		char error[1024];
        snprintf(error, 1024, "In %s(): %s.", func, message);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", error, NULL);
        exit(EXIT_FAILURE);
    }
}

//.. CONSTANTS
#define	GxCmaskNone &(Uint32) {0},
#define	GxCmaskAll &(Uint32) {~0},
#define	GxCmaskCamera &(Uint32){1u << 30}
#define	GxCmaskDynamic &(Uint32){1 << 0}
#define	GxCmaskFixed &(Uint32){1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7}

#define GxButtonKeyboard (1u << 0)
#define	GxButtonFinger (1u << 1)
#define	GxButtonMouse (1u << 2)
#define	GxButtonScreen (GxButtonFinger | GxButtonMouse)

#define	GxButtonNone (0u)
#define GxButtonOn (1u << 8)
#define	GxButtonHover (1u << 9)
#define	GxButtonClick (1u << 10)
#define	GxButtonDown (1u << 11)
#define	GxButtonUp  (1u << 12)


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


enum GxElemModuleType {
	GxDisplayAbsolute = 1 << 0,
	GxDisplayRelative = 1 << 1,
	GxDisplayNone = 1 << 2,
	GxBodyFixed = 1 << 3,
	GxBodyDynamic = 1 << 4,
	GxBodyNone = 1 << 6,
};

typedef enum GxElemOrientation {
	GxElemForward = SDL_FLIP_NONE,
	GxElemBackward = SDL_FLIP_HORIZONTAL,
}GxElemOrientation;


#endif // !GX_PUBLIC_H
