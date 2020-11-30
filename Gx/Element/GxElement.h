#ifndef GX_ELEM_H
#define GX_ELEM_H
#include "../Utilities/Util.h"
#include "../Ini/GxIni.h"
#include "../App/App.h"
#include "../Event/GxEvent.h"
#include <stdint.h>

//forward declaration
typedef struct GxElement GxElement;

//constructor, destructor and interface
GxElement* GxCreateElement(const GxIni* ini);
void GxDestroyElement_(GxElement* self);
void GxElemRemove(GxElement* self);


void* GxElemGetTarget(GxElement* self);
void GxElemAddRequestHandler(GxElement* self, 
	const char* request, GxRequestHandler handler
);
void GxElemDelegate(GxElement* self, const char* sceneReq, 
	GxElemID elem, const char* elemReq
);

Uint32 GxElemSceneGetId_(GxElement* self);

GxRequestData* GxElemGetRequestData_(GxElement* self, const char* request);

Uint32 GxElemGetId(GxElement* self);
const char* GxElemGetClassName(GxElement* self);
bool GxElemHasHandler(GxElement* self, int type);
GxHandler GxElemGetHandler(GxElement* self, int type);
void GxElemExecuteContactHandler_(GxElement* self, int type, GxContact* contact);

bool GxElemHasClass(GxElement* self, const char* type);
bool GxElemHasDynamicBody(GxElement* self);
bool GxElemHasFixedBody(GxElement* self);
bool GxElemHasRelativePosition(GxElement* self);
bool GxElemHasAbsolutePosition(GxElement* self);

GxScene* GxElemGetScene(GxElement* self);
const SDL_Rect* GxElemGetPosition(GxElement* self);
void GxElemSetPosition(GxElement* self, SDL_Rect pos);
GxPoint GxElemGetCenter(GxElement* self);

bool GxElemIsPhysical(GxElement* self);
bool GxElemIsRenderable(GxElement* self);

void GxElemSetChild(GxElement* self, void* child);
void* GxElemGetChild(GxElement* self);

#endif // !GX_ELEM_H

