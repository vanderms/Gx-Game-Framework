#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Util/Util.h"
#include "../App/App.h"

	
sScene* nSceneCreate(const sIni* ini);
Uint32 nSceneGetPercLoaded(sScene* self);
sSize nSceneSize(sScene* self);
bool nSceneHasStatus(sScene* self, int status);
int nSceneStatus(sScene* self);
sElement* nSceneGetElem(sScene* self, Uint32 id);
int nSceneGravity(sScene* self);
bool nSceneHasGravity(sScene* self);
const char* nSceneName(sScene* self);
void nSceneSetGravity(sScene* self, int gravity);
void nSceneSetTimeout(sScene* self, int interval, sHandler callback, void* target);
sElement* nSceneGetCamera(sScene* self);
void nSceneAddComponent(sScene* self, sComponent* comp);


void nSceneDestroy_(sScene* self);	
sPhysics* nSceneGetPhysics_(sScene* self);
sGraphics* nSceneGetGraphics_(sScene* self);	
Uint32 nSceneAddElem_(sScene* self, sElement* elem);
void nSceneRemoveElem_ (sScene* self, sElement* elem);
void nSceneSubscribeComponent_(sScene* self, sComponent* comp);
void nSceneUnsubscribeComponent_(sScene* self, sComponent* comp);
void nScenePreLoad_(sScene* self);
void nSceneUnLoad_(sScene* self);
void nSceneUpdate_(sScene* self);
void nSceneOnLoopBegin_(sScene* self);
void nSceneOnLoopEnd_(sScene* self);
void nSceneOnSDLEvent_(sScene* self, SDL_Event* e);
void nSceneOnPreContact_(sScene* self, sContact* contact);
void nSceneOnContactBegin_(sScene* self, sContact* contact);
void nSceneOnContactEnd_(sScene* self, sContact* contact);
	


#endif // !GX_SCENE_H


