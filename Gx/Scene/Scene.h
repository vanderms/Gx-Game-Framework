#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Utilities/Util.h"
#include "../Event/GxEvent.h"
#include "../App/App.h"

//struct declaration 
typedef struct sScene sScene;

//constructor and destructor
sScene* GxCreateScene(const sIni* ini);
void GxDestroyScene_(sScene* self);

//acessors and mutators
Uint32 GxSceneGetPercLoaded(sScene* self);
const char* GxSceneGetName(sScene* self);
void GxSceneExecuteElemChildDtor_(sScene* self, void* child);
sSize GxSceneGetSize(sScene* self);
bool GxSceneHasStatus(sScene* self, int status);
int GxSceneGetStatus(sScene* self);
sElement* GxSceneGetElement(sScene* self, Uint32 id);
int GxSceneGetGravity(sScene* self);
bool GxSceneHasGravity(sScene* self);
GxPhysics* GxSceneGetPhysics(sScene* self);
sGraphics* GxSceneGetGraphics(sScene* self);
sElement* GxSceneGetCamera(sScene* self);
void GxSceneSetGravity(sScene* self, int gravity);
void GxSceneSetTimeout(sScene* self, int interval, sHandler callback, void* target);
Uint32 GxSceneAddElement_(sScene* self, sElement* elem);
void GxSceneRemoveElement_ (sScene* self, sElement* elem);
void GxSceneSubscribeElemListeners_(sScene* self, sElement* elem);
void GxSceneUnsubscribeElemListeners_(sScene* self, sElement* elem);
void GxSceneAddEventListener(sScene* self, int type, sHandler handler, void* target);
bool GxSceneRemoveEventListener(sScene* self, int type, sHandler handler, void* target);

void GxScenePreLoad_(sScene* self);
void GxSceneUnload_(sScene* self);
void GxSceneOnUpdate_(sScene* self);
void GxSceneOnLoopBegin_(sScene* self);
void GxSceneOnLoopEnd_(sScene* self);

void GxSceneOnSDLEvent_(sScene* self, SDL_Event* e);
void GxSceneOnPreContact_(sScene* self, GxContact* contact);
void GxSceneOnContactBegin_(sScene* self, GxContact* contact);
void GxSceneOnContactEnd_(sScene* self, GxContact* contact);

#endif // !GX_SCENE_H


