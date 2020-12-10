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
	nUtil->assertAlloc(self);
	self->hash = nUtil->hash->ELEMENT;

	//set type, scene and target 
	self->className = ini->className ? nUtil->createString(ini->className) : NULL;
	self->classList = ini->className ? nUtil->split(ini->className, "|") : NULL;
	self->scene = nAppGetRunningScene();
	
	self->components = NULL;
	self->comp = nComponent->create(ini);
	if (self->comp){
		if(!self->comp->target) {
			self->comp->target = self;
		}
		nScene->p_->subscribeComponent(self->scene, self->comp);		
	}	

	
	//set position
	if (ini->display != nElem_DISPLAY_NONE || ini->body != nElem_BODY_NONE) {
		nUtil->assertArgument(ini->position);
		self->pos = malloc(sizeof(sRect));
		nUtil->assertAlloc(self->pos);
		*self->pos = *ini->position; 
	}
	else {
		self->pos = NULL;
	}
			
	self->body = nElemCreateBody_(self, ini);
	self->renderable = nElemCreateRenderable_(self, ini);	

	//add element to scene then return
	self->id = nScene->p_->addElem(self->scene, self);
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
						.type = nComponent->ON_DESTROY
					});
				}
				nScene->p_->unsubscribeComponent(self->scene, comp);
			}
			nMapDestroy(self->components);
		}
		
		if(self->comp){
			if(self->comp->onDestroy){
				self->comp->onDestroy(&(sEvent){
					.target = self->comp->target, 
					.type = nComponent->ON_DESTROY, 
				});
			}		
			nScene->p_->unsubscribeComponent(self->scene, self->comp);
		}
		nComponent->destroy(self->comp);
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
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);	
	nScene->p_->removeElem(self->scene, self);
}

void* nElemGetComponent(sElement* self, const char* name) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if (name) {
		nUtil->assertImplementation(self->components != NULL);
		sComponent* comp = nMapGet(self->components, name);
		nUtil->assertImplementation(comp != NULL);
		return comp->target;
	}
	return (self->comp ? self->comp->target : self);	
}

Uint32 nElemPId_(sElement* self) {
	return self->id;
}

Uint32 nElemId(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->id;
}

const char* nElemClassName(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->className;
}


void nElemExecuteHandler_(sElement* self, sEvent* ev){
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if (self->comp) {
		sHandler handler = nComponent->getHandler(self->comp, ev->type);
		if(handler){
			ev->target = self->comp->target;
			handler(ev);
		}
	}
	if (self->components) {
		for (Uint32 i = 0; i < nMapSize(self->components); i++) {
			sComponent* comp = nMapAt(self->components, i);
			sHandler handler = nComponent->getHandler(comp, ev->type);
			if(handler){
				ev->target = comp->target;
				handler(ev);
			}
		}
	}	
}


void nElemAddComponent(sElement* self, sComponent* comp) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertArgument(comp->name && !nComponent->isComponentEmpty(comp));
	sComponent* copy = nComponent->copy(comp);
	if (!self->components) {
		self->components = nMapCreate();
	}
	nMapSet(self->components, comp->name, copy, nComponent->destroy);
	nScene->p_->subscribeComponent(self->scene, copy);
}

bool nElemHasClass(sElement* self, const char* type) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
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
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->scene;
}

const sRect* nElemPosition(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->pos;
}

const sRect* nElemPosGetter_(void* value){
	sElement* self = value;
	return self->pos;
};

void nElemSetPosition(sElement* self, sRect pos) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	sGraphics* graphics = nScene->p_->getGraphics(self->scene);
	sPhysics* physics = nScene->p_->getPhysics(self->scene);
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
		nGraphicsUpdateElement_(nScene->p_->getGraphics(self->scene), self, previousPos);
	}
	if(self->body){
		nPhysicsUpdateElem_(nScene->p_->getPhysics(self->scene), self, previousPos);
	}
}

sPoint nElemCalcCenter(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (sPoint) {self->pos->x + self->pos->w/2, self->pos->y + self->pos->h/2};
}

bool nElemHasBody(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (bool) self->body;
}

bool nElemIsRenderable(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (bool) self->renderable;
}


struct sElemBody* nElemBody_(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertImplementation(self->body);
	return self->body;
}

struct sElemRenderable* nElemRenderable_(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertImplementation(self->renderable);
	return self->renderable;
}

void nElemSetBody_(sElement* self, struct sElemBody* body) {
	self->body = body;
};

void nElemSetRenderable_(sElement* self, struct sElemRenderable* renderable){
	self->renderable = renderable;
}
