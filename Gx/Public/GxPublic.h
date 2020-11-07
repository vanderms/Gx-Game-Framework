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
typedef struct GxArray GxArray;
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

typedef union GxData {
	int i;
	Uint32 u;
	char c;
	bool b;
	double f;	
	const char* s;
	const void* ptr;
	GxList* list;
	GxArray* array;
	GxMap* map;
	SDL_Rect rect;	
	GxVector vector;
	GxSize size;
	SDL_Point point;
	GxMatrix matrix;
} GxData;

typedef struct GxRequest {
	void* target;
	const char* request;
	GxData* data;
} GxRequest;

typedef struct GxResponse {
	GxData* value;
}GxResponse;

//...ASSERTIONS
#define GxAssertNullPointer(pointer) GxAssertion_((pointer), "Null Pointer")
#define GxAssertInvalidArgument(condition) GxAssertion_((condition), "Invalid argument")
#define GxAssertAllocationFailure(pointer) GxAssertion_((pointer), "Failed to allocate memory")
#define GxAssertNotImplemented(condition) GxAssertion_((condition), "Trying to access not implemented module")
#define GxAssertInvalidHash(condition) GxAssertion_((condition), "Invalid hash")
#define GxAssertOutOfRange(condition) GxAssertion_((condition), "Out of range")
#define GxAssertInvalidOperation(condition) GxAssertion_((condition), "Invalid operation")
#define GxAssertNotFoundAsset(condition) GxAssertion_((condition), "Not found asset");

static inline void GxAssertion_(bool condition, const char* error) {
	if (!condition) {		
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error, NULL);
		SDL_TriggerBreakpoint();
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
