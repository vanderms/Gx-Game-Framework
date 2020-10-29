#ifndef GX_NAMESPACE_H
#define GX_NAMESPACE_H
#include "Namespace/GxNamespace.h"
#include "XMacros/GxXMacros.h"

//... CONSTANTS: MACRO
#define	CmaskNone GxCmaskNone
#define	CmaskAll GxCmaskAll
#define	CmaskCamera GxCmaskCamera
#define	CmaskDynamic GxCmaskDynamic
#define	CmaskFixed GxCmaskFixed

#define ButtonKeyboard GxButtonKeyboard
#define	ButtonFinger GxButtonFinger
#define	ButtonMouse GxButtonMouse

#define	ButtonNone GxButtonNone
#define ButtonOn GxButtonOn
#define	ButtonHover GxButtonHover
#define	ButtonClick GxButtonClick
#define	ButtonDown GxButtonDown
#define	ButtonUp GxButtonUp

#define assertNullPointer GxAssertNullPointer
#define assertInvalidArgument GxAssertInvalidArgument
#define assertAllocationFailure GxAssertAllocationFailure
#define assertNotImplemented GxAssertNotImplemented
#define assertInvalidHash GxAssertInvalidHash
#define assertOutOfRange GxAssertOutOfRange
#define assertInvalidOperation GxAssertInvalidOperation


//...CONSTANTS:: ENUM
enum GxConstantsAliases {
	//... contact
	ContactRight = GxContactRight,
	ContactLeft = GxContactLeft,
	ContactHorizontal = GxContactHorizontal,
	ContactUp = GxContactUp,
	ContactDown = GxContactDown,
	ContactVertical = GxContactVertical,
	ContactAll = GxContactAll,

	//...status
	StatusNone = GxStatusNone,
	StatusLoading = GxStatusLoading,
	StatusLoaded = GxStatusLoaded,
	StatusRunning = GxStatusRunning,
	StatusPaused = GxStatusPaused,
	StatusReady = GxStatusReady,
	StatusUnloading = GxStatusUnloading,

	//..events
	EventOnLoad = GxEventOnLoad,
	EventOnLoopBegin = GxEventOnLoopBegin,
	EventOnUpdate = GxEventOnUpdate,
	EventOnPreGraphical = GxEventOnPreGraphical,
	EventOnPreRender = GxEventOnPreRender,
	EventOnLoopEnd = GxEventOnLoopEnd,
	EventOnUnload = GxEventOnUnload,
	EventOnKeyboard = GxEventOnKeyboard,
	EventOnMouse = GxEventMouse,
	EventOnFinger = GxEventFinger,
	EventOnSDLDefault = GxEventSDLDefault,
	EventOnPreContact = GxEventPreContact,
	EventOnContactBegin = GxEventContactBegin,
	EventOnContactEnd = GxEventContactEnd,
	EventOnTimeout = GxEventTimeout,
	EventOnDestroy = GxEventOnDestroy,
	EventOnElemRemoval = GxEventOnElemRemoval,

	//.. elem
	DisplayAbsolute = GxDisplayAbsolute,
	DisplayRelative = GxDisplayRelative,
	DisplayNone = GxDisplayNone,
	BodyFixed = GxBodyFixed,
	BodyDynamic = GxBodyDynamic,
	BodyNone = GxBodyNone,
	ElemForward = GxElemForward,
	ElemBackward = GxElemBackward,
};

//... TYPES ALIASES
typedef SDL_Rect Rect;
typedef SDL_Point Point;
typedef SDL_Point Vector;
typedef GxSize Size;
typedef GxMatrix Matrix;
typedef GxIni Ini;
typedef GxList List;
typedef GxArray Array;
typedef GxMap Map;
typedef GxEvent Event;
typedef GxRequest Request;
typedef GxScene Scene;
typedef GxElement Element;
typedef GxContact Contact;

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

//... NAMESPACES ALIASES
static const GxAppNamespace* app = &GxAppNamespaceInstance;
static const GxArrayNamespace* array = &GxArrayNamespaceInstance;
static const GxButtonNamespace* button = &GxButtonNamespaceInstance;
static const GxElemNamespace* elem = &GxElemNamespaceInstance;
static const GxFolderNamespace* folder = &GxFolderNamespaceInstance;
static const GxListNamespace* list =  &GxListNamespaceInstance;
static const GxMapNamespace* map = &GxMapNamespaceInstance;
static const GxContactNamespace* contact = &GxContactNamespaceInstance;
static const GxSceneNamespace* scene = &GxSceneNamespaceInstance;
static const GxUtilNamespace* util = &GxUtilNamespaceInstance;

#ifdef __GNUC__
   #pragma GCC diagnostic pop
#endif

static const int teste = 0;

#endif // !GX_NAMESPACE_H
