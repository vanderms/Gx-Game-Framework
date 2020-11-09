#ifndef GX_NAMESPACE_H
#define GX_NAMESPACE_H
#include "Namespace/GxNamespace.h"
#include "XMacros/GxXMacros.h"

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
typedef GxResponse Response;
typedef GxScene Scene;
typedef GxElement Element;
typedef GxContact Contact;
typedef GxData Data;
typedef GxElemID ElemID;

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

#endif // !GX_NAMESPACE_H
