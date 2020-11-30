#include "../Utilities/Util.h"
#include "../Ini/GxIni.h"
#include "../Scene/GxScene.h"
#include "../App/App.h"
#include "../Element/GxElement.h"
#include "../Event/GxEvent.h"
#include "../RigidBody/GxRigidBody.h"
#include "../Map/GxMap.h"
#include "../List/GxList.h"
#include "../Graphics/GxGraphics.h"
#include "../Physics/GxPhysics.h"
#include "../Folder/GxFolder.h"
#include "../Event/GxEvent.h"
#include <string.h>

typedef struct GxScene {
	Uint32 hash;
	char* name;	
	GxSize size;	
	int status;
	GxGraphics* graphics;
	GxPhysics* physics;	
	int gravity;
	GxElement* camera;
	sArray* elements;	
	sArray* folders;
	GxList* listeners[GxEventTotalHandlers];
	bool freeze;	

	//callbacks	
	void* target;
	GxHandler* handlers;	
	GxMap* rHandlers;


	//...
	Uint32 elemCounter;
} GxScene;

typedef struct Timer {
	int counter;
	void* target;		
	GxHandler callback;	
} Timer;

typedef struct Listener {
	GxEvent e;
	GxHandler handler;	
}Listener;

//constructor and destructor
GxScene* GxCreateScene(const GxIni* ini) {
	
	nsUtil->assertState(nsApp->isCreated());

	GxScene* self = calloc(1, sizeof(GxScene));	
	nsUtil->assertAlloc(self);
	self->hash = nsUtil->hash->SCENE;

	//set id
	if (ini->name){ 
		self->name = nsUtil->createString(ini->name); 
	}	
	else {
		static unsigned long int unique = 0;
		char buffer[64];
		snprintf(buffer, 64, "AUTO::SCENE::%lu", unique++);
		self->name = nsUtil->createString(buffer);
	}
	
	GxSize windowSize = nsApp->logicalSize();
	self->size.w = ini->size.w > windowSize.w ? ini->size.w : windowSize.w;
	self->size.h = ini->size.h > windowSize.h ? ini->size.h : windowSize.h;
	self->status = GxStatusNone;
	self->gravity = ini->gravity > 0 ? -ini->gravity : ini->gravity;	

	//set callback module
	self->target = ini->target ? ini->target : self;

	//event handler module
	if (GxEventIniHasHandler_(ini)) {
		self->handlers = calloc(GxEventTotalHandlers, sizeof(GxHandler));
		nsUtil->assertAlloc(self->handlers);
		GxEventSetHandlers_(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
	}	
	
	self->rHandlers = NULL;
	self->elemCounter = 0;

	//set folders
	if (ini->folders) {
		self->folders = nsUtil->split(ini->folders, "|");
		for (Uint32 i = 0; i < nsArr->size(self->folders); i++) {
			const char* folderId = nsArr->at(self->folders, i);
			GxFolder* folder = nsApp->prv->getFolder(folderId);
			nsUtil->assertArgument(folder);
			nsArr->insert(self->folders, i, folder, NULL);
		}
	}	
	//...
	
	nsApp->prv->addScene(self);
	
	return self;
}

void GxDestroyScene_(GxScene* self) {

	if (self) {
		self->status = GxStatusUnloading;
		
		for(Listener* listener = GxListBegin(self->listeners[GxEventOnDestroy]); 
			listener != NULL; listener = GxListNext(self->listeners[GxEventOnDestroy]))
		{		
			listener->handler(&(GxEvent) {
				.target = listener->e.target,
				.type = GxEventOnDestroy,				
			});			
		}
		
		GxDestroyGraphics_(self->graphics);
		GxDestroyPhysics_(self->physics);
		nsArr->destroy(self->elements);
		GxDestroyMap(self->rHandlers);
		
		if (self->handlers && self->handlers[GxEventOnDestroy]){
			self->handlers[GxEventOnDestroy](&(GxEvent) {
				.target = self->target,
				.type = GxEventOnDestroy,
			});
		}	

		if(self->folders){
			for (Uint32 i = 0; i < nsArr->size(self->folders); i++) {
				GxFolder* folder = nsArr->at(self->folders, i);
				GxFolderDecRefCounter_(folder);
			}
			nsArr->destroy(self->folders);		
		}
		//events
		for (int i = 0; i < GxEventTotalHandlers; i++) {
			GxDestroyList(self->listeners[i]);		
		}
		free(self->handlers);
		free(self->name);
		self->hash = 0;
		free(self);
	}
}


void GxSceneAddRequestHandler(GxScene* self, 
	const char* request, GxRequestHandler handler
){	
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	nsUtil->assertNullPointer(request);
	nsUtil->assertNullPointer(handler);
	GxRequestData* data = GxCreateRequestData_(self->target, request, handler);
	if (self->rHandlers == NULL) {
		self->rHandlers = GmCreateMap();
	}
	GxMapSet(self->rHandlers, request, data, GxDestroyRequestData_);
}

void GxSceneDelegate(GxScene* self, const char* sceneReq, 
	GxElemID elem, const char* elemReq
){
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	nsUtil->assertNullPointer(sceneReq);
	nsUtil->assertNullPointer(elemReq);
	GxElement* element = GxSceneGetElement(self, elem);
	nsUtil->assertArgument(element != NULL);
	GxRequestData* data = GxElemGetRequestData_(element, elemReq);
	nsUtil->assertArgument(data != NULL);
	if (self->rHandlers == NULL) {
		self->rHandlers = GmCreateMap();
	}
	GxMapSet(self->rHandlers, sceneReq, data, NULL);
}


const char* GxSceneGetName(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->name;
}

GxSize GxSceneGetSize(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->size;
}

GxElement* GxSceneGetCamera(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->camera;
}

GxPhysics* GxSceneGetPhysics(GxScene* self) {	
	return self->physics;
}

GxGraphics* GxSceneGetGraphics(GxScene* self) {
	return self->graphics;
}

bool GxSceneHasStatus(GxScene* self, int status) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->status == status;
}

int GxSceneGetStatus(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->status;
}

GxElement* GxSceneGetElement(GxScene* self, Uint32 id) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	if (id > nsArr->size(self->elements)) {
		return NULL;
	}
	Uint32 l = 0;
	Uint32 r = nsArr->size(self->elements);
	
	while (l <= r) { 
        Uint32 m = l + (r - l) / 2; 		
		       
		GxElement* elem = nsArr->at(self->elements, m);
		Uint32 elemID = GxElemSceneGetId_(elem);
        if (elemID == id){
            return elem; 
        }
        else if (elemID < id){
            l = m + 1; 
        }
        else {
            r = m - 1; 
		}
    }
	
    return NULL;
}

int GxSceneGetGravity(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->gravity;
}

bool GxSceneHasGravity(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	return self->gravity;
}

void GxSceneSetGravity(GxScene* self, int gravity) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	self->gravity = gravity > 0 ? -gravity : gravity;
}

Uint32 GxSceneGetPercLoaded(GxScene* self) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	
	if (!self->folders || !nsArr->size(self->folders)) {
		return 100;
	}

	Uint32 total = 0;
	for (Uint32 i = 0; i < nsArr->size(self->folders); i++){	
		GxFolder* folder = nsArr->at(self->folders, i);
		total += GXFolderGetPercLoaded_(folder);
	}
	
	return total / nsArr->size(self->folders);
}

void GxSceneExecuteElemChildDtor_(GxScene* self, void* child) {
	
	if(self->status == GxStatusUnloading){return ;}

	for(Listener* listener = GxListBegin(self->listeners[GxEventOnDestroy]);
		listener != NULL; listener = GxListNext(self->listeners[GxEventOnDestroy])) 
	{
		if (listener->e.target == child) {
			listener->handler(&(GxEvent) {
				.target = listener->e.target,
				.type = GxEventOnDestroy,				
			});
			GxListRemove(self->listeners[GxEventOnDestroy], listener);
		}
	}
}

void GxSceneSetTimeout(GxScene* self, int interval, GxHandler callback, void* target) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	Timer* timer = malloc(sizeof(Timer));
	nsUtil->assertAlloc(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	GxListPush(self->listeners[GxEventTimeout], timer, free);
}

Uint32 GxSceneAddElement_(GxScene* self, GxElement* elem) {	
	nsUtil->assertState(self->status == GxStatusLoaded || self->status == GxStatusRunning);	
	nsArr->push(self->elements, elem, (GxDestructor) GxDestroyElement_);	
	GxGraphicsInsertElement_(self->graphics, elem);
	GxPhysicsInsertElement_(self->physics, elem);
	GxSceneSubscribeElemListeners_(self, elem);
	Uint32 id = self->elemCounter++;
	return id;
}


void GxSceneSubscribeElemListeners_(GxScene* self, GxElement* elem) {
	for (int i = 0; i < GxEventTotalHandlers; i++) {
		if(i == GxEventOnDestroy){ continue; }
		GxHandler handler = GxElemGetHandler(elem, i);
		if (handler) {
			void* target = GxElemGetTarget(elem);
			GxSceneAddEventListener(self, i, handler, target);
		}
	}
}

void GxSceneUnsubscribeElemListeners_(GxScene* self, GxElement* elem) {
	
	for (int i = 0; i < GxEventTotalHandlers; i++) {
		GxHandler handler = GxElemGetHandler(elem, i);
		if (handler) {
			void* target = GxElemGetTarget(elem);
			GxSceneRemoveEventListener(self, i, handler, target);
		}	
	}
}

void GxSceneAddEventListener(GxScene* self, int type, GxHandler handler, void* target) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	nsUtil->assertArgument(type >= 0 && type < GxEventTotalHandlers);
	nsUtil->assertNullPointer(handler);
	nsUtil->assertNullPointer(target);
	Listener* listener = malloc(sizeof(Listener));
	nsUtil->assertAlloc(listener);
	listener->e.target = target;
	listener->e.type = type;
	listener->handler = handler;
	GxListPush(self->listeners[type], listener, free);
}

bool GxSceneRemoveEventListener(GxScene* self, int type, GxHandler handler, void* target) {
	nsUtil->assertNullPointer(self);
	nsUtil->assertHash((*(Uint32*) self) == nsUtil->hash->SCENE);
	if (self->status == GxStatusUnloading) { return true; }

	nsUtil->assertArgument(type >= 0 && type < GxEventTotalHandlers);
	nsUtil->assertNullPointer(handler);
	nsUtil->assertNullPointer(target);
	for(Listener* listener = GxListBegin(self->listeners[type]);
		listener != NULL; listener = GxListNext(self->listeners[type])) {
		if (listener->handler == handler && listener->e.target == target) {
			GxListRemove(self->listeners[type], listener);
			return true;
		}
	}
	return false;
}

static inline void sceneExecuteListeners(GxScene* self, int type, SDL_Event* sdle) {	
		
	if (self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent) {
			.target = self->target,
			.type = type,	
			.sdle = sdle
		});
	}
	
	for (Listener* listener = GxListBegin(self->listeners[type]); listener != NULL;
		listener = GxListNext(self->listeners[type]))
	{			
		listener->e.sdle = sdle;
		listener->handler(&listener->e);		
	}
}

static inline void sceneExecuteContactListeners(GxScene* self, int type, GxContact* contact) {
	GxElement* elemSelf = GxContactGetColliding(contact);
	GxElement* elemOther = GxContactGetCollided(contact);
	GxElemSetMcFlag_(elemSelf, true);
	GxElemSetMcFlag_(elemOther, true);

	if (self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent) {
			.target = self->target,
			.type = type,	
			.contact = contact
		});
	}
	GxElemExecuteContactHandler_(elemSelf, type, contact);
	GxElemExecuteContactHandler_(elemOther, type, contact);
	
	GxElemSetMcFlag_(elemSelf, false);
	GxElemSetMcFlag_(elemOther, false);
}

void GxScenePreLoad_(GxScene* self) {
	
	nsUtil->assertState(self->status == GxStatusNone);

	self->status = GxStatusLoading;
	self->elemCounter = 0;

	//initialize containers and folders
	self->graphics = GxCreateGraphics_(self);
	self->physics = GxCreatePhysics_(self);
	self->elements = nsArr->create();		
		
	for (int i = 0; i < GxEventTotalHandlers; i++) {
		self->listeners[i] = GxCreateList();
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < nsArr->size(self->folders); i++) {		
			GxFolderIncRefCounter_(nsArr->at(self->folders, i));
		}
	}
}

static void GxSceneLoad_(GxScene* self) {		
	
	//create physic walls and camera
	GxPhysicsCreateWalls_(self->physics);
	GxSize size = nsApp->logicalSize();
	self->camera = GxCreateElement(&(GxIni) {
		.display = GxElemNone,
		.body = GxElemFixed,
		.className = "__CAMERA__",		
		.position = &(SDL_Rect){ 0, 0, size.w, size.h },				
	});

	GxElemSetCmask(self->camera, GxCmaskCamera);
	
	//execute onload callbacks 
	sceneExecuteListeners(self, GxEventOnLoad, NULL);

	//change status to running
	self->status = GxStatusRunning;
}

void GxSceneUnload_(GxScene* self) {
	self->status = GxStatusUnloading;
	GxDestroyGraphics_(self->graphics);
	GxDestroyPhysics_(self->physics);	
	//delete buttons_;
	
	sceneExecuteListeners(self, GxEventOnUnload, NULL);
	
	for(Listener* listener = GxListBegin(self->listeners[GxEventOnDestroy]); 
		listener != NULL; listener = GxListNext(self->listeners[GxEventOnDestroy]))
	{		
		listener->handler(&(GxEvent) {
			.target = listener->e.target,
			.type = GxEventOnDestroy,				
		});			
	}
	nsArr->destroy(self->elements);	
	
	if(self->folders){
		for (Uint32 i = 0; i< nsArr->size(self->folders); i++){
			GxFolder* folder = nsArr->at(self->folders, i);			
			GxFolderDecRefCounter_(folder);
		}	
	}
			
	for (int i = 0; i < GxEventTotalHandlers; i++) {
		GxDestroyList(self->listeners[i]);
		self->listeners[i] = NULL;
	}
			
	self->graphics = NULL;
	self->physics = NULL;
	self->elements = NULL;	
	self->camera = NULL;
	
	//status		
	self->status = GxStatusNone;
}

void GxSceneRemoveElement_(GxScene* self, GxElement* elem) {	
	if (GxElemIsPhysical(elem)) GxPhysicsRemoveElement_(self->physics, elem);
	if (GxElemIsRenderable(elem)) GxGraphicsRemoveElement_(self->graphics, elem);
	if (elem != self->camera) GxSceneUnsubscribeElemListeners_(self, elem);
	nsArr->removeByValue(self->elements, elem);	
}

void GxSceneOnLoopBegin_(GxScene* self) {	
	if(self->status == GxStatusRunning){
		sceneExecuteListeners(self, GxEventOnLoopBegin, NULL);		
	}
}

void GxSceneOnUpdate_(GxScene* self) {
	
	if (self->status == GxStatusLoading) {
		if (GxSceneGetPercLoaded(self) == 100) {
			self->status = GxStatusLoaded;
			GxSceneLoad_(self);
		}		
	}

	if(self->status == GxStatusRunning){

		for (Timer* timer = GxListBegin(self->listeners[GxEventTimeout]); timer != NULL;
			timer = GxListNext(self->listeners[GxEventTimeout])
		){						
			if (--timer->counter <= 0) {
				timer->callback(&(GxEvent){
					.target = timer->target,
					.type = GxEventTimeout
				});	
				GxListRemove(self->listeners[GxEventTimeout], timer);
			}
		}
	
		//execute update callbacks, then update physics
		sceneExecuteListeners(self, GxEventOnUpdate, NULL);
		GxPhysicsUpdate_(self->physics);	

		//execute preGraphical callbacks, then update graphics
		sceneExecuteListeners(self, GxEventOnPreGraphical, NULL);
		GxGraphicsUpdate_(self->graphics);

		//execute prerender callbacks 
		sceneExecuteListeners(self, GxEventOnPreRender, NULL);
	}
}

void GxSceneOnLoopEnd_(GxScene* self) {
	if(self->status == GxStatusRunning){
		sceneExecuteListeners(self, GxEventOnLoopEnd, NULL);
	}
}

void GxSceneOnSDLEvent_(GxScene* self, SDL_Event* e) {	
	if (self->status != GxStatusRunning) {
		return;
	}

	switch (e->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:			
			sceneExecuteListeners(self, GxEventOnKeyboard, e);
			break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:			
			sceneExecuteListeners(self, GxEventMouse, e);
			break;
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_MULTIGESTURE:
		case SDL_DOLLARGESTURE:			
			sceneExecuteListeners(self, GxEventFinger, e);
			break;
		default:		
			sceneExecuteListeners(self, GxEventSDLDefault, e);
			break;
	}	
}

void GxSceneOnPreContact_(GxScene* self, GxContact* contact) {
	sceneExecuteContactListeners(self, GxEventPreContact, contact);
}

void GxSceneOnContactBegin_(GxScene* self, GxContact* contact) {
	sceneExecuteContactListeners(self, GxEventContactBegin, contact);
}

void GxSceneOnContactEnd_(GxScene* self, GxContact* contact) {
	sceneExecuteContactListeners(self, GxEventContactEnd, contact);
}