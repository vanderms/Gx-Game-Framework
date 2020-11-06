#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Utilities/GxUtil.h"
#include "../Event/GxEvent.h"
#include "../App/GxApp.h"

//struct declaration 
typedef struct GxScene GxScene;

//constructor and destructor
GxScene* GxCreateScene(const GxIni* ini);
void GxDestroyScene_(GxScene* self);

//acessors and mutators
GxData* GxSceneSend(GxScene* self, const char* description, GxData* data);
Uint32 GxSceneGetPercLoaded(GxScene* self);
const char* GxSceneGetName(GxScene* self);
void GxSceneExecuteElemChildDtor_(GxScene* self, void* child);
void GxSceneAddRequestHandler(GxScene* receiver, 
	const char* request, GxRequestHandler handler
);
GxSize GxSceneGetSize(GxScene* self);
bool GxSceneHasStatus(GxScene* self, int status);
int GxSceneGetStatus(GxScene* self);
GxElement* GxSceneGetElement(GxScene* self, Uint32 id);
int GxSceneGetGravity(GxScene* self);
bool GxSceneHasGravity(GxScene* self);
GxPhysics* GxSceneGetPhysics(GxScene* self);
GxGraphics* GxSceneGetGraphics(GxScene* self);
GxElement* GxSceneGetCamera(GxScene* self);
void GxScenePause(GxScene* self);
void GxSceneResume(GxScene* self);
void GxSceneSetGravity(GxScene* self, int gravity);
void GxSceneSetTimeout(GxScene* self, int interval, GxHandler callback, void* target);
Uint32 GxSceneAddElement_(GxScene* self, GxElement* elem);
void GxSceneRemoveElement_ (GxScene* self, GxElement* elem);
void GxSceneSubscribeElemListeners_(GxScene* self, GxElement* elem);
void GxSceneUnsubscribeElemListeners_(GxScene* self, GxElement* elem);
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


