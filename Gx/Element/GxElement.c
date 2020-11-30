#include "../Utilities/Util.h"
#include "../Private/GxElement.h"
#include "../Element/GxElement.h"
#include "../Event/GxEvent.h"
#include "../Array/Array.h"
#include "../Folder/GxFolder.h"
#include "../Graphics/GxGraphics.h"
#include "../Physics/GxPhysics.h"
#include "../Scene/GxScene.h"
#include <string.h>
#include "../Renderable/GxRenderable.h"
#include "../Map/GxMap.h"



GxElement* GxCreateElement(const GxIni* ini){

	GxElement* self = malloc(sizeof(GxElement));
	nsUtil->assertAlloc(self);
	self->hash = nsUtil->hash->ELEMENT;	

	//set type, scene and target 
	self->className = ini->className ? nsUtil->createString(ini->className) : NULL;
	self->classList = ini->className ? nsUtil->split(ini->className, "|") : NULL;
	self->scene = nsApp->getRunningScene();
	self->target = ini->target ? ini->target : self;
	self->child = NULL;

	if (ini->target) {
		nsUtil->assertArgument(ini->onDestroy);
	}
		
	//event handler module
	if (GxEventIniHasHandler_(ini)) {
		self->handlers = calloc(GxEventTotalHandlers, sizeof(GxHandler));
		nsUtil->assertAlloc(self->handlers);
		GxEventSetHandlers_(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
	}
			
	self->body = GxCreateRigidBody_(self, ini);
	self->renderable = GxCreateRenderable_(self, ini);

	//set position
	if (self->renderable || self->body) {
		nsUtil->assertArgument(ini->position);
		self->pos = malloc(sizeof(SDL_Rect));
		nsUtil->assertAlloc(self->pos);
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

void GxDestroyElement_(GxElement* self) {	
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
		nsArr->destroy(self->classList);
		self->hash = 0;
		free(self->handlers);
		free(self->pos);
		free(self->className);		
		free(self);
	}
}

void GxElemRemove(GxElement* self) {
	validateElem(self, false, false);
	GxSceneRemoveElement_(self->scene, self);
}

void* GxElemGetTarget(GxElement* self) {
	validateElem(self, false, false);
	return self->target;
}

void GxElemAddRequestHandler(GxElement* self, 
	const char* request, GxRequestHandler handler
){
	validateElem(self, false, false);	
	nsUtil->assertNullPointer(request);
	nsUtil->assertNullPointer(handler);
	GxRequestData* data = GxCreateRequestData_(self->target, request, handler);
	if (self->rHandlers == NULL) {
		self->rHandlers = GmCreateMap();
	}
	GxMapSet(self->rHandlers, request, data, GxDestroyRequestData_);
}

GxRequestData* GxElemGetRequestData_(GxElement* self, const char* request) {
	return self->rHandlers ? GxMapGet(self->rHandlers, request) : NULL;	
}

void GxElemDelegate(GxElement* self, const char* sceneReq, 
	GxElemID elem, const char* elemReq
){
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	nsUtil->assertNullPointer(sceneReq);
	nsUtil->assertNullPointer(elemReq);
	GxElement* other = GxSceneGetElement(self->scene, elem);
	nsUtil->assertArgument(other != NULL);
	GxRequestData* data = GxElemGetRequestData_(self, elemReq);
	nsUtil->assertArgument(data != NULL);
	if (self->rHandlers == NULL) {
		self->rHandlers = GmCreateMap();
	}
	GxMapSet(self->rHandlers, sceneReq, data, NULL);
}

Uint32 GxElemSceneGetId_(GxElement* self) {
	return self->id;
}

Uint32 GxElemGetId(GxElement* self) {
	validateElem(self, false, false);
	return self->id;
}

const char* GxElemGetClassName(GxElement* self) {
	validateElem(self, false, false);
	return self->className;
}

bool GxElemHasHandler(GxElement* self, int type) {
	validateElem(self, false, false);
	return self->handlers && self->handlers[type] ? true : false;
}

GxHandler GxElemGetHandler(GxElement* self, int type) {
	validateElem(self, false, false);
	return self->handlers ? self->handlers[type] : NULL;
}

void GxElemExecuteContactHandler_(GxElement* self, int type, GxContact* contact){
	validateElem(self, false, false);
	if(self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent){
			.type = type, 
			.target = self->target,
			.contact = contact,			
		});
	}
}

bool GxElemHasClass(GxElement* self, const char* type) {
	validateElem(self, false, false);	
	if (!self->classList) {
		return false;
	}

	sArray* types = nsApp->tokenize(type, "|");
	Uint32 matches = 0;
		
	for(Uint32 i = 0; i < nsArr->size(types); i++){
		for (Uint32 j = 0; j < nsArr->size(self->classList); j++) {
			char* token = nsArr->at(self->classList, j);
			if(strcmp(token, nsArr->at(types, i)) == 0){
				matches++;
			}
		}		
	}
	return matches == nsArr->size(types);
}

GxScene* GxElemGetScene(GxElement* self) {
	validateElem(self, false, false);
	return self->scene;
}

const SDL_Rect* GxElemGetPosition(GxElement* self) {
	validateElem(self, false, false);
	return self->pos;
}

void GxElemSetPosition(GxElement* self, SDL_Rect pos) {
	validateElem(self, false, false);
	GxGraphics* graphics = GxSceneGetGraphics(self->scene);
	GxPhysics* physics = GxSceneGetPhysics(self->scene);
	if (self->renderable) GxGraphicsRemoveElement_(graphics, self);
	if (self->body) GxPhysicsRemoveElement_(physics, self);
	*self->pos = pos;
	if (self->renderable)GxGraphicsInsertElement_(graphics, self);
	if (self->body) GxPhysicsInsertElement_(physics, self);	
}

GxPoint GxElemGetCenter(GxElement* self) {
	validateElem(self, false, false);
	return (GxPoint) {self->pos->x + self->pos->w/2, self->pos->y + self->pos->h/2};
}

bool GxElemIsPhysical(GxElement* self) {
	validateElem(self, false, false);
	return (bool) self->body;
}

bool GxElemIsRenderable(GxElement* self) {
	validateElem(self, false, false);
	return (bool) self->renderable;
}

void GxElemSetChild(GxElement* self, void* child) {
	validateElem(self, false, false);
	nsUtil->assertState(!self->child);
	self->child = child;
}

void* GxElemGetChild(GxElement* self) {
	validateElem(self, false, false);
	return self->child;
}
