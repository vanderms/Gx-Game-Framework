#ifndef GX_H
#define GX_H
#include "../App/App.h"
#include "../Element/Element.h"
#include "../Folder/Folder.h"




















/*

typedef struct GxContactNamespace {	
	sElement* (*getColliding)(sContact* contact);
	sElement* (*getCollided)(sContact* contact);
	bool (*isBetween)(sContact* contact, sElement* self, sElement* other);
	bool (*hasElement)(sContact* contact, sElement* element);
	bool (*hasDirection)(sContact* contact, Uint32 direction);
	Uint32 (*getDirection)(sContact* contact);
	sElement* (*getOppositeElement)(sContact* contact, sElement* self);	
	void (*allowCollision)(sContact* contact);
	bool (*isElemRightContact)(sContact* contact, sElement* self);
	bool (*isElemLeftContact)(sContact* contact, sElement* self);
	bool (*isElemDownContact)(sContact* contact, sElement* self);
	bool (*isElemUpContact)(sContact* contact, sElement* self);
	void (*oneWayPlatform)(sEvent* e);
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
*/

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
extern const GxSceneNamespace GxSceneNamespaceInstance;
#endif // !GX_NAMESPACE_H

