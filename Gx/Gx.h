#ifndef GX_NAMESPACE_H
#define GX_NAMESPACE_H
#include "Namespace/GxNamespace.h"
#include "Components/Components.h"
#include "XMacros/GxXMacros.h"

//... TYPES ALIASES
typedef sRect Rect;
typedef SDL_Point Point;
typedef SDL_Point Vector;
typedef sSize Size;
typedef sMatrix Matrix;
typedef sIni sIni;
typedef sList List;
typedef sArray Array;
typedef sMap Map;
typedef sEvent Event;
typedef sScene Scene;
typedef sElement sElement;
typedef sContact Contact;
typedef sElemId ElemID;

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

//... NAMESPACES ALIASES
static const GxSceneNamespace* scene = &GxSceneNamespaceInstance;

#ifdef __GNUC__
   #pragma GCC diagnostic pop
#endif

#endif // !GX_NAMESPACE_H
