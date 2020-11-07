#include "../Utilities/GxUtil.h"
#include "../Ini/GxIni.h"
#include "../Scene/GxScene.h"
#include "../App/GxApp.h"
#include "../Element/GxElement.h"
#include "../RigidBody/GxRigidBody.h"
#include "../Map/GxMap.h"
#include "../List/GxList.h"
#include "../Graphics/GxGraphics.h"
#include "../Physics/GxPhysics.h"
#include "../Folder/GxFolder.h"
#include "../Button/GxButton.h"
#include "../Event/GxEvent.h"
#include <string.h>


typedef struct GxScene {
	char* name;	
	GxSize size;	
	int status;
	GxGraphics* graphics;
	GxPhysics* physics;	
	int gravity;
	GxElement* camera;
	GxArray* elements;	
	GxArray* folders;
	GxList* listeners[GxEventTotalHandlers];
	bool freeze;	

	//callbacks	
	void* target;
	GxHandler* handlers;	
	GxMap* rHandlers;
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
	
	GxAssertInvalidOperation(GxAppIsCreated_());

	GxScene* self = calloc(1, sizeof(GxScene));	
	GxAssertAllocationFailure(self);

	//set id
	if (ini->name){ 
		self->name = GmCreateString(ini->name); 
	}	
	else {
		static unsigned long int unique = 0;
		char buffer[64];
		snprintf(buffer, 64, "AUTO::SCENE::%lu", unique++);
		self->name = GmCreateString(buffer);
	}
	
	GxSize windowSize = GxGetWindowSize();
	self->size.w = ini->size.w > windowSize.w ? ini->size.w : windowSize.w;
	self->size.h = ini->size.h > windowSize.h ? ini->size.h : windowSize.h;
	self->status = GxStatusNone;
	self->gravity = ini->gravity > 0 ? -ini->gravity : ini->gravity;	

	//set callback module
	self->target = ini->target ? ini->target : self;

	//event handler module
	if (GxEventIniHasHandler_(ini)) {
		self->handlers = calloc(GxEventTotalHandlers, sizeof(GxHandler));
		GxAssertAllocationFailure(self->handlers);
		GxEventSetHandlers_(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
	}	
	
	self->rHandlers = NULL;

	//set folders
	if (ini->folders) {
		self->folders = GmArraySplit(ini->folders, "|");
		for (Uint32 i = 0; i < GxArraySize(self->folders); i++) {
			const char* folderId = GxArrayAt(self->folders, i);
			GxFolder* folder = GxGetFolder_(folderId);
			GxAssertInvalidArgument(folder);
			GxArrayInsert(self->folders, i, folder, NULL);
		}
	}	
	//...
	
	GxAddScene_(self);
	
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
		GxDestroyArray(self->elements);
		GxDestroyMap(self->rHandlers);
		
		if (self->handlers && self->handlers[GxEventOnDestroy]){
			self->handlers[GxEventOnDestroy](&(GxEvent) {
				.target = self->target,
				.type = GxEventOnDestroy,
			});
		}	
		if(self->folders){
			for (Uint32 i = 0; i < GxArraySize(self->folders); i++) {
				GxFolder* folder = GxArrayAt(self->folders, i);
				GxFolderDecRefCounter_(folder);
			}
			GxDestroyArray(self->folders);		
		}
		//events
		for (int i = 0; i < GxEventTotalHandlers; i++) {
			GxDestroyList(self->listeners[i]);		
		}
		free(self->handlers);
		free(self->name);
		free(self);
	}
}


void GxSceneAddRequestHandler(GxScene* self, 
	const char* request, GxRequestHandler handler
){	
	if (self->rHandlers == NULL) {
		self->rHandlers = GmCreateMap();
	}
	GxMapSet(self->rHandlers, request, handler, NULL);
}


GxData* GxSceneSend(GxScene* receiver, const char* request, GxData* data) {
	GxRequestHandler handler = GxMapGet(receiver->rHandlers, request);
	GxAssertNotImplemented(handler);
	GxResponse response = {.value = NULL};
	handler(&(GxRequest){ receiver->target, request, data }, &response);
	return response.value;
}


const char* GxSceneGetName(GxScene* self) {
	return self->name;
}

GxSize GxSceneGetSize(GxScene* self) {
	return self->size;
}

GxElement* GxSceneGetCamera(GxScene* self) {
	return self->camera;
}

GxPhysics* GxSceneGetPhysics(GxScene* self) {
	return self->physics;
}

GxGraphics* GxSceneGetGraphics(GxScene* self) {
	return self->graphics;
}

bool GxSceneHasStatus(GxScene* self, int status) {
	return self->status == status;
}

int GxSceneGetStatus(GxScene* self) {
	return self->status;
}

GxElement* GxSceneGetElement(GxScene* self, Uint32 id) {
	if (id > GxArraySize(self->elements)) {
		return NULL;
	}
	else return GxArrayAt(self->elements, id);   
}

int GxSceneGetGravity(GxScene* self) {
	return self->gravity;
}

bool GxSceneHasGravity(GxScene* self) {
	return self->gravity;
}

void GxScenePause(GxScene* self) {
	if (self->status == GxStatusRunning) {
		self->status = GxStatusPaused;
	}
}

void GxSceneResume(GxScene* self) {
	if (self->status == GxStatusPaused) {
		self->status = GxStatusRunning;
	}
}

void GxSceneSetGravity(GxScene* self, int gravity) {
	self->gravity = gravity > 0 ? -gravity : gravity;
}

Uint32 GxSceneGetPercLoaded(GxScene* self) {
	
	if (!self->folders || !GxArraySize(self->folders)) {
		return 100;
	}

	Uint32 total = 0;
	for (Uint32 i = 0; i < GxArraySize(self->folders); i++){	
		GxFolder* folder = GxArrayAt(self->folders, i);
		total += GXFolderGetPercLoaded_(folder);
	}
	
	return total / GxArraySize(self->folders);
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
	Timer* timer = malloc(sizeof(Timer));
	GxAssertAllocationFailure(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	GxListPush(self->listeners[GxEventTimeout], timer, free);
}

Uint32 GxSceneAddElement_(GxScene* self, GxElement* elem) {
	GxAssertInvalidOperation(self->status == GxStatusLoaded || self->status == GxStatusRunning);	
	GxArrayPush(self->elements, elem, (GxDestructor) GxDestroyElement_);	
	GxGraphicsInsertElement_(self->graphics, elem);
	GxPhysicsInsertElement_(self->physics, elem);
	GxSceneSubscribeElemListeners_(self, elem);
	Uint32 id = GxArraySize(self->elements) - 1;
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
	GxAssertInvalidArgument(type >= 0 && type < GxEventTotalHandlers);
	GxAssertNullPointer(handler && target);
	Listener* listener = malloc(sizeof(Listener));
	GxAssertAllocationFailure(listener);
	listener->e.target = target;
	listener->e.type = type;
	listener->handler = handler;
	GxListPush(self->listeners[type], listener, free);
}

bool GxSceneRemoveEventListener(GxScene* self, int type, GxHandler handler, void* target) {
	
	if (self->status == GxStatusUnloading) { return true; }

	GxAssertInvalidArgument(type >= 0 && type < GxEventTotalHandlers);
	GxAssertNullPointer(handler && target);
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
	
	GxAssertInvalidOperation(self->status == GxStatusNone);

	self->status = GxStatusLoading;

	//initialize containers and folders
	self->graphics = GxCreateGraphics_(self);
	self->physics = GxCreatePhysics_(self);
	self->elements = GxCreateArray();		
		
	for (int i = 0; i < GxEventTotalHandlers; i++) {
		self->listeners[i] = GxCreateList();
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < GxArraySize(self->folders); i++) {		
			GxFolderIncRefCounter_(GxArrayAt(self->folders, i));
		}
	}
}

static void GxSceneLoad_(GxScene* self) {		
	
	//create physic walls and camera
	GxPhysicsCreateWalls_(self->physics);
	GxSize size = GxGetWindowSize();
	self->camera = GxCreateElement(&(GxIni) {
		.display = GxElemNone,
		.body = GxElemFixed,
		.className = "__CAMERA__",		
		.position = &(SDL_Rect){ 0, 0, size.w, size.h },		
		.cmask = GxCmaskCamera
	});
	
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
	GxDestroyArray(self->elements);	
	
	if(self->folders){
		for (Uint32 i = 0; i< GxArraySize(self->folders); i++){
			GxFolder* folder = GxArrayAt(self->folders, i);			
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
	GxArrayInsert(self->elements, GxElemGetId(elem), NULL, NULL);	
}

void GxSceneOnLoopBegin_(GxScene* self) {	
	sceneExecuteListeners(self, GxEventOnLoopBegin, NULL);		
}

void GxSceneOnUpdate_(GxScene* self) {
	
	if (self->status == GxStatusLoading) {
		if (GxSceneGetPercLoaded(self) == 100) {
			self->status = GxStatusLoaded;
			GxSceneLoad_(self);
		}
		else {	
			return;
		}
	}

	for (Timer* timer = GxListBegin(self->listeners[GxEventTimeout]); timer != NULL;
		timer = GxListNext(self->listeners[GxEventTimeout]))
	{						
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

void GxSceneOnLoopEnd_(GxScene* self) {
	sceneExecuteListeners(self, GxEventOnLoopEnd, NULL);
}

void GxSceneOnSDLEvent_(GxScene* self, SDL_Event* e) {	

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
