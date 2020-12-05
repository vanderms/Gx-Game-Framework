#include "../Utilities/Util.h"
#include "Element.h"
#include "../Array/Array.h"
#include "../Folder/Folder.h"
#include "../Graphics/Graphics.h"
#include "../Physics/Physics.h"
#include "../Scene/Scene.h"
#include <string.h>
#include "../Map/Map.h"

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

	//event handler module
	void* target;
	sHandler* handlers;	
	sMap* rHandlers;

	//special elements
	void* child;
	
} sElement;

static sElement* create(const sIni* ini){

	sElement* self = malloc(sizeof(sElement));
	nUtil->assertAlloc(self);
	self->hash = nUtil->hash->ELEMENT;	

	//set type, scene and target 
	self->className = ini->className ? nUtil->createString(ini->className) : NULL;
	self->classList = ini->className ? nUtil->split(ini->className, "|") : NULL;
	self->scene = nApp->getRunningScene();
	self->target = ini->target ? ini->target : self;
	self->child = NULL;

	if (ini->target) {
		nUtil->assertArgument(ini->onDestroy);
	}
		
	//event handler module
	if (nUtil->evn->hasHandler(ini)) {
		self->handlers = calloc(nUtil->evn->TOTAL, sizeof(sHandler));
		nUtil->assertAlloc(self->handlers);
		nUtil->evn->setHandlers(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
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
			
	self->body = nElem->body->p->create(self, ini);
	self->renderable = nElem->style->p->create(self, ini);

	//getHandler and putHandler		
	self->rHandlers = NULL;

	//add element to scene then return
	self->id = nScene->p->addElem(self->scene, self);
	return self;
}

static void pDestroy(sElement* self) {	
	if (self) {
		if (self->child) {
			nScene->p->executeCompDtor(self->scene, self->child);
		}
		if(self->handlers && self->handlers[nUtil->evn->ON_DESTROY]){
			self->handlers[nUtil->evn->ON_DESTROY](&(sEvent){
				.target = self->target, 
				.type = nUtil->evn->ON_DESTROY, 
			});
		}
		nMap->destroy(self->rHandlers);
		nElem->body->p->destroy(self->body);
		nElem->style->p->destroy(self->renderable);
		nArray->destroy(self->classList);
		self->hash = 0;
		free(self->handlers);
		free(self->pos);
		free(self->className);
		free(self);
	}
}

static void removeElement(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);	
	nScene->p->removeElem(self->scene, self);
}

static void* target(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->target;
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

static bool hasHandler(sElement* self, int type) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->handlers && self->handlers[type] ? true : false;
}

static sHandler getHandler(sElement* self, int type) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->handlers ? self->handlers[type] : NULL;
}

static void pExecuteContactHandler(sElement* self, int type, sContact* contact){
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if(self->handlers && self->handlers[type]){
		self->handlers[type](&(sEvent){
			.type = type, 
			.target = self->target,
			.contact = contact,			
		});
	}
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
	sGraphics* graphics = nScene->p->getGraphics(self->scene);
	sPhysics* physics = nScene->p->getPhysics(self->scene);
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
		nGraphics->updateElement(nScene->p->getGraphics(self->scene), self, previousPos);
	}
	if(self->body){
		nPhysics->updateElem(nScene->p->getPhysics(self->scene), self, previousPos);
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

const struct sElemNamespace* nElem = &(struct sElemNamespace){
	.create = create,	
	.remove = removeElement,
	.target = target,
		
	.id = id,
	.className = className,
	.hasHandler = hasHandler,
	.getHandler = getHandler,	

	.hasClass = hasClass,

	.scene = scene,
	.position = position,
	.setPosition = setPosition,
	.calcCenter = calcCenter,

	.hasBody = hasBody,
	.isRenderable = isRenderable,

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
	.p = &(struct sElemPrivateNamespace){
		.destroy = pDestroy,
		.id = pId,
		.posGetter = posGetter,
		.executeContactHandler = pExecuteContactHandler,
		.body = pBody,
		.renderable = pRenderable,
		.setBody = pSetBody,
		.setRenderable = pSetRenderable,
		.updatePosition = pUpdatePosition,
	},
};