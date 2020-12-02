#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Utilities/Util.h"
#include "../Event/GxEvent.h"
#include "../App/App.h"

//struct declaration 
typedef struct GxScene GxScene;

//constructor and destructor
GxScene* GxCreateScene(const sIni* ini);
void GxDestroyScene_(GxScene* self);

//acessors and mutators
Uint32 GxSceneGetPercLoaded(GxScene* self);
const char* GxSceneGetName(GxScene* self);
void GxSceneExecuteElemChildDtor_(GxScene* self, void* child);
void GxSceneAddRequestHandler(GxScene* receiver, 
	const char* request, GxRequestHandler handler
);
void GxSceneDelegate(GxScene* self, const char* sceneReq, 
	GxElemID elem, const char* elemReq
);
GxSize GxSceneGetSize(GxScene* self);
bool GxSceneHasStatus(GxScene* self, int status);
int GxSceneGetStatus(GxScene* self);
sElement* GxSceneGetElement(GxScene* self, Uint32 id);
int GxSceneGetGravity(GxScene* self);
bool GxSceneHasGravity(GxScene* self);
GxPhysics* GxSceneGetPhysics(GxScene* self);
GxGraphics* GxSceneGetGraphics(GxScene* self);
sElement* GxSceneGetCamera(GxScene* self);
void GxSceneSetGravity(GxScene* self, int gravity);
void GxSceneSetTimeout(GxScene* self, int interval, GxHandler callback, void* target);
Uint32 GxSceneAddElement_(GxScene* self, sElement* elem);
void GxSceneRemoveElement_ (GxScene* self, sElement* elem);
void GxSceneSubscribeElemListeners_(GxScene* self, sElement* elem);
void GxSceneUnsubscribeElemListeners_(GxScene* self, sElement* elem);
void GxSceneAddEventListener(GxScene* self, int type, GxHandler handler, void* target);
bool GxSceneRemoveEventListener(GxScene* self, int type, GxHandler handler, void* target);

void GxScenePreLoad_(GxScene* self);
void GxSceneUnload_(GxScene* self);
void GxSceneOnUpdate_(GxScene* self);
void GxSceneOnLoopBegin_(GxScene* self);
void GxSceneOnLoopEnd_(GxScene* self);

void GxSceneOnSDLEvent_(GxScene* self, SDL_Event* e);
void GxSceneOnPreContact_(GxScene* self, GxContact* contact);
void GxSceneOnContactBegin_(GxScene* self, GxContact* contact);
void GxSceneOnContactEnd_(GxScene* self, GxContact* contact);

#endif // !GX_SCENE_H


