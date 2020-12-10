#include "../Util/Util.h"
#include "Element.h"
#include "../Containers/Array/Array.h"
#include "../Folder/Folder.h"
#include "../Graphics/Graphics.h"
#include "../Physics/Physics.h"
#include "../Scene/Scene.h"
#include <string.h>
#include "../Containers/Map/Map.h"

typedef struct sElement {
	
	//main data
	uint32_t hash;
	Uint32 id;	
	char* className;
	sArray* classList;
	sScene* scene;
	sRect* pos;
	
	//modules
	struct sElemRenderable* renderable;
	struct sElemBody* body;

	//components
	sComponent* comp;
	sMap* components;
	
} sElement;

sElement* nElemCreate(const sIni* ini){

	sElement* self = malloc(sizeof(sElement));
	nUtilAssertAlloc(self);
	self->hash = nUtil_HASH_ELEMENT;

	//set type, scene and target 
	self->className = ini->className ? nUtilCreateString(ini->className) : NULL;
	self->classList = ini->className ? nUtilSplit(ini->className, "|") : NULL;
	self->scene = nAppGetRunningScene();
	
	self->components = NULL;
	self->comp = nComponentCreate_(ini);
	if (self->comp){
		if(!self->comp->target) {
			self->comp->target = self;
		}
		nSceneSubscribeComponent_(self->scene, self->comp);		
	}	

	
	//set position
	if (ini->display != nElem_DISPLAY_NONE || ini->body != nElem_BODY_NONE) {
		nUtilAssertArgument(ini->position);
		self->pos = malloc(sizeof(sRect));
		nUtilAssertAlloc(self->pos);
		*self->pos = *ini->position; 
	}
	else {
		self->pos = NULL;
	}
			
	self->body = nElemCreateBody_(self, ini);
	self->renderable = nElemCreateRenderable_(self, ini);	

	//add element to scene then return
	self->id = nSceneAddElem_(self->scene, self);
	return self;
}

void nElemDestroy_(sElement* self) {	
	if (self) {
		if (self->components) {
			for (Uint32 i = 0; i < nMapSize(self->components); i++) {
				sComponent* comp = nMapAt(self->components, i);
				if (comp->onDestroy) {
					comp->onDestroy(&(sEvent) {
						.target = comp->target,
						.type = nEvent_ON_DESTROY
					});
				}
				nSceneUnsubscribeComponent_(self->scene, comp);
			}
			nMapDestroy(self->components);
		}
		
		if(self->comp){
			if(self->comp->onDestroy){
				self->comp->onDestroy(&(sEvent){
					.target = self->comp->target, 
					.type = nEvent_ON_DESTROY, 
				});
			}		
			nSceneUnsubscribeComponent_(self->scene, self->comp);
		}
		nComponentDestroy_(self->comp);
		nElemDestroyBody_(self->body);
		nElemDestroyRenderable_(self->renderable);
		nArrayDestroy(self->classList);				
		free(self->pos);
		free(self->className);
		self->hash = 0;
		free(self);
	}
}

void nElemRemove(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);	
	nSceneRemoveElem_(self->scene, self);
}

void* nElemGetComponent(sElement* self, const char* name) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	if (name) {
		nUtilAssertImplementation(self->components != NULL);
		sComponent* comp = nMapGet(self->components, name);
		nUtilAssertImplementation(comp != NULL);
		return comp->target;
	}
	return (self->comp ? self->comp->target : self);	
}

Uint32 nElemPId_(sElement* self) {
	return self->id;
}

Uint32 nElemId(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return self->id;
}

const char* nElemClassName(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return self->className;
}


void nElemExecuteHandler_(sElement* self, sEvent* ev){
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	if (self->comp) {
		sHandler handler = nComponentGetHandler_(self->comp, ev->type);
		if(handler){
			ev->target = self->comp->target;
			handler(ev);
		}
	}
	if (self->components) {
		for (Uint32 i = 0; i < nMapSize(self->components); i++) {
			sComponent* comp = nMapAt(self->components, i);
			sHandler handler = nComponentGetHandler_(comp, ev->type);
			if(handler){
				ev->target = comp->target;
				handler(ev);
			}
		}
	}	
}


void nElemAddComponent(sElement* self, sComponent* comp) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	nUtilAssertArgument(comp->name && !nComponentIsEmpty_(comp));
	sComponent* copy = nComponentCopy_(comp);
	if (!self->components) {
		self->components = nMapCreate();
	}
	nMapSet(self->components, comp->name, copy, nComponentDestroy_);
	nSceneSubscribeComponent_(self->scene, copy);
}

bool nElemHasClass(sElement* self, const char* type) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	if (!self->classList) {
		return false;
	}

	sArray* types = nAppTokenize(type, "|");
	Uint32 matches = 0;
		
	for(Uint32 i = 0; i < nArraySize(types); i++){
		for (Uint32 j = 0; j < nArraySize(self->classList); j++) {
			char* token = nArrayAt(self->classList, j);
			if(strcmp(token, nArrayAt(types, i)) == 0){
				matches++;
			}
		}		
	}
	return matches == nArraySize(types);
}

sScene* nElemScene(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return self->scene;
}

const sRect* nElemPosition(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return self->pos;
}

const sRect* nElemPosGetter_(void* value){
	sElement* self = value;
	return self->pos;
}

void nElemSetPosition(sElement* self, sRect pos) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	sGraphics* graphics = nSceneGetGraphics_(self->scene);
	sPhysics* physics = nSceneGetPhysics_(self->scene);
	if (self->renderable) nGraphicsRemoveElement_(graphics, self);
	if (self->body) nPhysicsRemoveElem_(physics, self);
	*self->pos = pos;
	if (self->renderable) nGraphicsInsert_(graphics, self);
	if (self->body) nPhysicsInsert_(physics, self);	
}

void nElemUpdatePosition_(sElement* self, sVector vector) {	
	sRect previousPos = *self->pos;
	self->pos->x += vector.x;
	self->pos->y += vector.y;
	if(self->renderable){
		nGraphicsUpdateElement_(nSceneGetGraphics_(self->scene), self, previousPos);
	}
	if(self->body){
		nPhysicsUpdateElem_(nSceneGetPhysics_(self->scene), self, previousPos);
	}
}

sPoint nElemCalcCenter(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return (sPoint) {self->pos->x + self->pos->w/2, self->pos->y + self->pos->h/2};
}

bool nElemHasBody(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return (bool) self->body;
}

bool nElemIsRenderable(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	return (bool) self->renderable;
}


struct sElemBody* nElemBody_(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	nUtilAssertImplementation(self->body);
	return self->body;
}

struct sElemRenderable* nElemRenderable_(sElement* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_ELEMENT);
	nUtilAssertImplementation(self->renderable);
	return self->renderable;
}

void nElemSetBody_(sElement* self, struct sElemBody* body) {
	self->body = body;
}

void nElemSetRenderable_(sElement* self, struct sElemRenderable* renderable){
	self->renderable = renderable;
}
