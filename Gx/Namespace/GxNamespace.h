#ifndef GX_H
#define GX_H
#include "../Public/GxPublic.h"
#include "../App/App.h"
#include "../Element/Element.h"
#include "../Folder/Folder.h"

//... NAMESPACES


struct sEventNamespace {
	const int LOAD;
	const int LOOP_BEGIN;
	const int UPDATE;
	const int ON_RENDER;	
	const int LOOP_END;
	const int UNLOAD;
	const int KEYBOARD;
	const int MOUSE;
	const int FINGER;
	const int SDL_DEFAULT;
	const int PRE_CONTACT;
	const int CONTACT_BEGIN;
	const int CONTACT_END;
	const int TIMEOUT;
	const int DESTROY;
	const int ELEM_REMOVAL;
};


















typedef struct GxMapNamespace {	
	GxMap* (*create)(void);
	void (*destroy)(GxMap* self);	
	Uint32 (*size)(GxMap* self);
	Uint32 (*capacity)(GxMap* self);
	void* (*get)(GxMap* self, const char* key);
	void* (*at)(GxMap* self, Uint32 index);
	void (*set)(GxMap* self, const char* key, void* value, sDtor dtor);
	void (*rehash)(GxMap* self, Uint32 capacity);
	void (*remove)(GxMap* self, const char* key);
	void (*removeByIndex)(GxMap* self, Uint32 index);
	void (*clean)(GxMap* self);
}GxMapNamespace;


typedef struct GxContactNamespace {	
	sElement* (*getColliding)(GxContact* contact);
	sElement* (*getCollided)(GxContact* contact);
	bool (*isBetween)(GxContact* contact, sElement* self, sElement* other);
	bool (*hasElement)(GxContact* contact, sElement* element);
	bool (*hasDirection)(GxContact* contact, Uint32 direction);
	Uint32 (*getDirection)(GxContact* contact);
	sElement* (*getOppositeElement)(GxContact* contact, sElement* self);	
	void (*allowCollision)(GxContact* contact);
	bool (*isElemRightContact)(GxContact* contact, sElement* self);
	bool (*isElemLeftContact)(GxContact* contact, sElement* self);
	bool (*isElemDownContact)(GxContact* contact, sElement* self);
	bool (*isElemUpContact)(GxContact* contact, sElement* self);
	void (*oneWayPlatform)(GxEvent* e);
	const GxContactConstant RIGHT;
	const GxContactConstant LEFT;
	const GxContactConstant HORIZONTAL;
	const GxContactConstant UP;
	const GxContactConstant DOWN;
	const GxContactConstant VERTICAL;
	const GxContactConstant ALL;
	const Uint32 CMASKNONE;
	const Uint32 CMASKALL;
	const Uint32 CMASKCAMERA;
	const Uint32 CMASKDYNAMIC;
	const Uint32 CMASKFIXED;
}GxContactNamespace;


struct GxStatusNamespace {
	const int NONE;
	const int LOADING;
	const int LOADED;
	const int RUNNING;	
	const int UNLOADING;
};

typedef struct GxSceneNamespace {
		
	sScene* (*create)(const sIni* ini);			
	Uint32 (*getPercLoaded)(sScene* self);
	const char* (*getName)(sScene* self);
	sSize (*getSize)(sScene* self);
	bool (*hasStatus)(sScene* self, int status);
	int (*getStatus)(sScene* self);
	sElement* (*getElem)(sScene* self, Uint32 id);
	int (*getGravity)(sScene* self);
	bool (*hasGravity)(sScene* self);
	sElement* (*getCamera)(sScene* self);	
	void (*setGravity)(sScene* self, int gravity);
	void (*setTimeout)(sScene* self, int interval, sHandler callback, void* target);	
	void (*addEventListener)(sScene* self, int type, sHandler handler, void* target);
	bool (*removeEventListener)(sScene* self, int type, sHandler handler, void* target);	
	const struct GxStatusNamespace* status;
}GxSceneNamespace;




//... INSTANCES
extern const GxMapNamespace GxMapNamespaceInstance;
extern const GxContactNamespace GxContactNamespaceInstance;
extern const GxSceneNamespace GxSceneNamespaceInstance;
#endif // !GX_NAMESPACE_H

