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
typedef SDL_Point sPoint;
typedef SDL_Point sVector;
typedef Uint32 GxElemID;

//... TYPES :: FORWARD DECLARATION
typedef struct sList sList;
typedef struct sArray sArray;
typedef struct GxMap GxMap;
typedef struct sIni sIni;
typedef struct GxIEventHandler GxIEventHandler;
typedef struct GxEvent GxEvent;
typedef struct sScene sScene;
typedef struct sElement sElement;
struct sElemRenderable;
struct sElemBody;
typedef struct sGraphics sGraphics;
typedef struct GxPhysics GxPhysics;
typedef struct GxContact GxContact;
typedef struct sImage sImage;
typedef struct sAnimation sAnimation;
typedef struct sFolder sFolder;
typedef struct sChunk sChunk;
typedef struct sMusic sMusic;

//... FUNCTIONS :: ALIAS
typedef void (*sDtor)();
typedef int (*GxComp)(const void*, const void*);
typedef void (*sHandler)(GxEvent*);

//... TYPES :: DEFINITIONS
typedef struct sSize {
	int w;
	int h;
} sSize;

typedef struct sMatrix {
	int nr;
	int nc;
} sMatrix;

typedef struct GxEvent {
	void* target;
	int type;
	union { GxContact* contact; SDL_Event* sdle; };
} GxEvent;


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


#endif // !GX_PUBLIC_H
