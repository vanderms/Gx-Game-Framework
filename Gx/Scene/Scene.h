#ifndef GX_SCENE_H
#define GX_SCENE_H
#include "../Utilities/Util.h"
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
	void (*addListener)(sScene* self, int type, sHandler handler, void* target);
	bool (*removeListener)(sScene* self, int type, sHandler handler, void* target);

	const struct sScenePrivateNamespace {
		void (*destroy)(sScene* self);
		void (*executeCompDtor)(sScene* self, void* child);	
		sPhysics* (*getPhysics)(sScene* self);
		sGraphics* (*getGraphics)(sScene* self);	
		Uint32 (*addElem)(sScene* self, sElement* elem);
		void (*removeElem) (sScene* self, sElement* elem);
		void (*subsElemListeners)(sScene* self, sElement* elem);
		void (*unsubsElemListeners)(sScene* self, sElement* elem);
		void (*preLoad)(sScene* self);
		void (*unLoad)(sScene* self);
		void (*update)(sScene* self);
		void (*onLoopBegin)(sScene* self);
		void (*onLoopEnd)(sScene* self);
		void (*onSDLEvent)(sScene* self, SDL_Event* e);
		void (*onPreContact)(sScene* self, sContact* contact);
		void (*onContactBegin)(sScene* self, sContact* contact);
		void (*onContactEnd)(sScene* self, sContact* contact);
	}* p;	
}* nScene;

#endif // !GX_SCENE_H


