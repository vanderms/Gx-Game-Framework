#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Util/Util.h"
#include "../App/App.h"

extern const struct sSceneNamespace {
	
	sScene* (*create)(const sIni* ini);
	Uint32 (*getPercLoaded)(sScene* self);
	sSize (*size)(sScene* self);
	bool (*hasStatus)(sScene* self, int status);
	int (*status)(sScene* self);
	sElement* (*getElem)(sScene* self, Uint32 id);
	int (*gravity)(sScene* self);
	bool (*hasGravity)(sScene* self);
	const char* (*name)(sScene* self);
	void (*setGravity)(sScene* self, int gravity);
	void (*setTimeout)(sScene* self, int interval, sHandler callback, void* target);
	sElement* (*getCamera)(sScene* self);
	void (*addComponent)(sScene* self, sComponent* comp);

	const struct sScenePrivateNamespace {
		void (*destroy)(sScene* self);	
		sPhysics* (*getPhysics)(sScene* self);
		sGraphics* (*getGraphics)(sScene* self);	
		Uint32 (*addElem)(sScene* self, sElement* elem);
		void (*removeElem) (sScene* self, sElement* elem);
		void (*subscribeComponent)(sScene* self, sComponent* comp);
		void (*unsubscribeComponent)(sScene* self, sComponent* comp);
		void (*preLoad)(sScene* self);
		void (*unLoad)(sScene* self);
		void (*update)(sScene* self);
		void (*onLoopBegin)(sScene* self);
		void (*onLoopEnd)(sScene* self);
		void (*onSDLEvent)(sScene* self, SDL_Event* e);
		void (*onPreContact)(sScene* self, sContact* contact);
		void (*onContactBegin)(sScene* self, sContact* contact);
		void (*onContactEnd)(sScene* self, sContact* contact);
	}* p_;	
}* const nScene;

#endif // !GX_SCENE_H


