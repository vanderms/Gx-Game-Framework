#include "../Utilities/Util.h"
#include "Element.h"
#include "../Event/GxEvent.h"
#include "../Array/Array.h"
#include "../Folder/GxFolder.h"
#include "../Graphics/GxGraphics.h"
#include "../Physics/GxPhysics.h"
#include "../Scene/GxScene.h"
#include <string.h>
#include "../Map/GxMap.h"

typedef struct sElement {
	
	//main data
	uint32_t hash;
	Uint32 id;	
	char* className;
	sArray* classList;
	GxScene* scene;
	SDL_Rect* pos;
	
	//modules
	struct sElemRenderable* renderable;
	struct sElemBody* body;

	//event handler module
	void* target;
	GxHandler* handlers;	
	GxMap* rHandlers;

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
	if (GxEventIniHasHandler_(ini)) {
		self->handlers = calloc(GxEventTotalHandlers, sizeof(GxHandler));
		nUtil->assertAlloc(self->handlers);
		GxEventSetHandlers_(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
	}
			
	self->body = GxCreateRigidBody_(self, ini);
	self->renderable = GxCreateRenderable_(self, ini);

	//set position
	if (self->renderable || self->body) {
		nUtil->assertArgument(ini->position);
		self->pos = malloc(sizeof(SDL_Rect));
		nUtil->assertAlloc(self->pos);
		*self->pos = *ini->position; 
	}
	else {
		self->pos = NULL;
	}

	//getHandler and putHandler		
	self->rHandlers = NULL;

	//add element to scene then return
	self->id = GxSceneAddElement_(self->scene, self);
	return self;
}

static void pDestroy(sElement* self) {	
	if (self) {
		if (self->child) {
			GxSceneExecuteElemChildDtor_(self->scene, self->child);
		}
		if(self->handlers && self->handlers[GxEventOnDestroy]){
			self->handlers[GxEventOnDestroy](&(GxEvent){
				.target = self->target, 
				.type = GxEventOnDestroy, 
			});
		}
		GxDestroyMap(self->rHandlers);
		GxDestroyRigidBody_(self->body);
		GxDestroyRenderable_(self->renderable);
		nArr->destroy(self->classList);
		self->hash = 0;
		free(self->handlers);
		free(self->pos);
		free(self->className);		
		free(self);
	}
}

static void remove(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);	
	GxSceneRemoveElement_(self->scene, self);
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

static GxHandler getHandler(sElement* self, int type) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->handlers ? self->handlers[type] : NULL;
}

static void pExecuteContactHandler(sElement* self, int type, GxContact* contact){
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	if(self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent){
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
		
	for(Uint32 i = 0; i < nArr->size(types); i++){
		for (Uint32 j = 0; j < nArr->size(self->classList); j++) {
			char* token = nArr->at(self->classList, j);
			if(strcmp(token, nArr->at(types, i)) == 0){
				matches++;
			}
		}		
	}
	return matches == nArr->size(types);
}

static GxScene* scene(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->scene;
}

static const SDL_Rect* position(sElement* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	return self->pos;
}

static void setPosition(sElement* self, SDL_Rect pos) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	GxGraphics* graphics = GxSceneGetGraphics(self->scene);
	GxPhysics* physics = GxSceneGetPhysics(self->scene);
	if (self->renderable) GxGraphicsRemoveElement_(graphics, self);
	if (self->body) GxPhysicsRemoveElement_(physics, self);
	*self->pos = pos;
	if (self->renderable)GxGraphicsInsertElement_(graphics, self);
	if (self->body) GxPhysicsInsertElement_(physics, self);	
}

static void pUpdatePosition(sElement* self, sVector vector) {	
	SDL_Rect previousPos = *self->pos;
	self->pos->x += vector.x;
	self->pos->y += vector.y;
	GxGraphicsUpdatePosition_(GxSceneGetGraphics(self->scene), self, previousPos);
	GxPhysicsUpdateElementPosition_(GxSceneGetPhysics(self->scene), self, previousPos);
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

extern const sElemNamespace* nElem = &(sElemNamespace){
	.create = create,	
	.remove = remove,
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

	.body = &nBodyNamespaceInstance,
	.style = &nRenderableNamespaceInstance,

	.p = &(struct sElemPrivateNamespace){
		.destroy = pDestroy,
		.id = pId,
		.executeContactHandler = pExecuteContactHandler,
		.body = pBody,
		.renderable = pRenderable,
		.setBody = pSetBody,
		.setRenderable = pSetRenderable,
		.updatePosition = pUpdatePosition,
	},
};