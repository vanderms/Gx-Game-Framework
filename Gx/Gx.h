#ifndef GX_NAMESPACE_H
#define GX_NAMESPACE_H
#include "Namespace/GxNamespace.h"
#include "Components/Components.h"
#include "XMacros/GxXMacros.h"

//... TYPES ALIASES
typedef SDL_Rect Rect;
typedef SDL_Point Point;
typedef SDL_Point Vector;
typedef GxSize Size;
typedef GxMatrix Matrix;
typedef GxIni Ini;
typedef GxList List;
typedef sArray Array;
typedef GxMap Map;
typedef GxEvent Event;
typedef GxRequest Request;
typedef GxResponse Response;
typedef GxScene Scene;
typedef GxElement Element;
typedef GxContact Contact;
typedef GxElemID ElemID;

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

//... NAMESPACES ALIASES
static const GxElemNamespace* elem = &GxElemNamespaceInstance;
static const GxFolderNamespace* folder = &GxFolderNamespaceInstance;
static const GxListNamespace* list =  &GxListNamespaceInstance;
static const GxMapNamespace* map = &GxMapNamespaceInstance;
static const GxContactNamespace* contact = &GxContactNamespaceInstance;
static const GxSceneNamespace* scene = &GxSceneNamespaceInstance;

#ifdef __GNUC__
   #pragma GCC diagnostic pop
#endif

#endif // !GX_NAMESPACE_H
