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

static sElement* create(const sIni* ini){

	sElement* self = malloc(sizeof(sElement));
	nUtil->assertAlloc(self);
	self->hash = nUtil->hash->ELEMENT;

	//set type, scene and target 
	self->className = ini->className ? nUtil->createString(ini->className) : NULL;
	self->classList = ini->className ? nUtil->split(ini->className, "|") : NULL;
	self->scene = nApp->getRunningScene();
	
	self->components = NULL;
	self->comp = nComponent->create(ini);
	if (self->comp){
		if(!self->comp->target) {
			self->comp->target = self;
		}
		nScene->p_->subscribeComponent(self->scene, self->comp);		
	}	

	
	//set position
	if (ini->display != nElem->display->NONE || ini->body != nElem->body->NONE) {
		nUtil->assertArgument(ini->position);
		self->pos = malloc(sizeof(sRect));
		nUtil->assertAlloc(self->pos);
		*self->pos = *ini->position; 
	}
	else {
		self->pos = NULL;
	}
			
	self->body = nElem->body->p_->create(self, ini);
	self->renderable = nElem->style->p_->create(self, ini);	

	//add element to scene then return
	self->id = nScene->p_->addElem(self->scene, self);
	return self;
}

static void pDestroy(sElement* self) {	
	if (self) {
		if (self->components) {
			for (Uint32 i = 0; i < nMap->size(self->components); i++) {
				sComponent* comp = nMap->at(self->components, i);
				if (comp->onDestroy) {
					comp->onDestroy(&(sEvent) {
						.target = comp->target,
						.type = nComponent->ON_DESTROY
					});
				}
				nScene->p_->unsubscribeComponent(self->scene, comp);
			}
			nMap->destroy(self->components);
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
		nElem->body->p_->destroy(self->body);
		nElem->style->p_->destroy(self->renderable);
		nArray->destroy(self->classList);				
		free(self->pos);
		free(self->className);
		self->hash = 0;
		free(self);
	}
}

static void removeElement(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);	
	nScene->p_->removeElem(self->scene, self);
}

static void* getComponent(sElement* self, const char* name) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if (name) {
		nUtil->assertImplementation(self->components != NULL);
		sComponent* comp = nMap->get(self->components, name);
		nUtil->assertImplementation(comp != NULL);
		return comp->target;
	}
	return (self->comp ? self->comp->target : self);	
}

static Uint32 pId(sElement* self) {
	return self->id;
}

static Uint32 id(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->id;
}

static const char* className(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->className;
}


static void pExecuteHandler(sElement* self, sEvent* ev){
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
		for (Uint32 i = 0; i < nMap->size(self->components); i++) {
			sComponent* comp = nMap->at(self->components, i);
			sHandler handler = nComponent->getHandler(comp, ev->type);
			if(handler){
				ev->target = comp->target;
				handler(ev);
			}
		}
	}	
}

static void addComponent(sElement* self, sComponent* comp) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertArgument(comp->name && !nComponent->isComponentEmpty(comp));
	sComponent* copy = nComponent->copy(comp);
	if (!self->components) {
		self->components = nMap->create();
	}
	nMap->set(self->components, comp->name, copy, nComponent->destroy);
	nScene->p_->subscribeComponent(self->scene, copy);
}

static bool hasClass(sElement* self, const char* type) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if (!self->classList) {
		return false;
	}

	sArray* types = nApp->tokenize(type, "|");
	Uint32 matches = 0;
		
	for(Uint32 i = 0; i < nArray->size(types); i++){
		for (Uint32 j = 0; j < nArray->size(self->classList); j++) {
			char* token = nArray->at(self->classList, j);
			if(strcmp(token, nArray->at(types, i)) == 0){
				matches++;
			}
		}		
	}
	return matches == nArray->size(types);
}

static sScene* scene(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->scene;
}

static const sRect* position(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->pos;
}

static const sRect* posGetter(void* value){
	sElement* self = value;
	return self->pos;
};

static void setPosition(sElement* self, sRect pos) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	sGraphics* graphics = nScene->p_->getGraphics(self->scene);
	sPhysics* physics = nScene->p_->getPhysics(self->scene);
	if (self->renderable) nGraphics->remove(graphics, self);
	if (self->body) nPhysics->remove(physics, self);
	*self->pos = pos;
	if (self->renderable) nGraphics->insert(graphics, self);
	if (self->body) nPhysics->insert(physics, self);	
}

static void pUpdatePosition(sElement* self, sVector vector) {	
	sRect previousPos = *self->pos;
	self->pos->x += vector.x;
	self->pos->y += vector.y;
	if(self->renderable){
		nGraphics->updateElement(nScene->p_->getGraphics(self->scene), self, previousPos);
	}
	if(self->body){
		nPhysics->updateElem(nScene->p_->getPhysics(self->scene), self, previousPos);
	}
}

static sPoint calcCenter(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (sPoint) {self->pos->x + self->pos->w/2, self->pos->y + self->pos->h/2};
}

static bool hasBody(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (bool) self->body;
}

static bool isRenderable(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return (bool) self->renderable;
}


static struct sElemBody* pBody(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertImplementation(self->body);
	return self->body;
}

static struct sElemRenderable* pRenderable(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertImplementation(self->renderable);
	return self->renderable;
}

static void pSetBody(sElement* self, struct sElemBody* body) {
	self->body = body;
};

static void pSetRenderable(sElement* self, struct sElemRenderable* renderable){
	self->renderable = renderable;
}

const struct sElemNamespace* const nElem = &(struct sElemNamespace){
	.create = create,	
	.remove = removeElement,
	
		
	.id = id,
	.className = className,
	
	.hasClass = hasClass,

	.scene = scene,
	.position = position,
	.setPosition = setPosition,
	.calcCenter = calcCenter,

	.hasBody = hasBody,
	.isRenderable = isRenderable,
	.addComponent = addComponent,
	.getComponent = getComponent,

	.body = &nElemBody,
	.style = &nElemRenderable,
	

	.display = &(struct sElemDisplay){
		.NONE = 1,
		.ABSOLUTE = 2,
		.RELATIVE = 3,	
	},

	.orientation = &(struct sElemOrientation){
		.FORWARD = SDL_FLIP_NONE,
		.BACKWARD = SDL_FLIP_HORIZONTAL,
	},
	.p_ = &(struct sElemPrivateNamespace){
		.destroy = pDestroy,
		.id = pId,
		.posGetter = posGetter,
		.executeHandler = pExecuteHandler,		
		.body = pBody,
		.renderable = pRenderable,
		.setBody = pSetBody,
		.setRenderable = pSetRenderable,
		.updatePosition = pUpdatePosition,
	},
};