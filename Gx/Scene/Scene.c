#include "../Utilities/Util.h"
#include "../Scene/Scene.h"
#include "../App/App.h"
#include "../Element/Element.h"
#include "../Event/GxEvent.h"
#include "../Map/Map.h"
#include "../List/List.h"
#include "../Graphics/Graphics.h"
#include "../Physics/Physics.h"
#include "../Folder/Folder.h"
#include "../Event/GxEvent.h"
#include <string.h>

typedef struct sScene {
	Uint32 hash;
	char* name;	
	sSize size;	
	int status;
	sGraphics* graphics;
	sPhysics* physics;	
	int gravity;
	sElement* camera;
	sArray* elements;	
	sArray* folders;
	sList** listeners;
	bool freeze;	

	//callbacks	
	void* target;
	sHandler* handlers;	
	sMap* rHandlers;


	//...
	Uint32 elemCounter;
} sScene;

typedef struct Timer {
	int counter;
	void* target;		
	sHandler callback;	
} Timer;

typedef struct Listener {
	GxEvent e;
	sHandler handler;	
}Listener;

//constructor and destructor
sScene* GxCreateScene(const sIni* ini) {
	
	nUtil->assertState(nApp->isCreated());

	sScene* self = calloc(1, sizeof(sScene));	
	nUtil->assertAlloc(self);
	self->hash = nUtil->hash->SCENE;

	//set id
	if (ini->name){ 
		self->name = nUtil->createString(ini->name); 
	}	
	else {
		static unsigned long int unique = 0;
		char buffer[64];
		snprintf(buffer, 64, "AUTO::SCENE::%lu", unique++);
		self->name = nUtil->createString(buffer);
	}
	
	sSize windowSize = nApp->logicalSize();
	self->size.w = ini->size.w > windowSize.w ? ini->size.w : windowSize.w;
	self->size.h = ini->size.h > windowSize.h ? ini->size.h : windowSize.h;
	self->status = GxStatusNone;
	self->gravity = ini->gravity > 0 ? -ini->gravity : ini->gravity;	

	//set callback module
	self->target = ini->target ? ini->target : self;

	//event handler module
	if (nUtil->evn->hasHandler(ini)) {
		self->handlers = calloc(nUtil->evn->TOTAL, sizeof(sHandler));
		nUtil->assertAlloc(self->handlers);
		nUtil->evn->setHandlers(self->handlers, ini);
	}
	else {
		self->handlers = NULL;
	}	

	self->listeners = calloc(nUtil->evn->TOTAL, sizeof(sList*));
	nUtil->assertAlloc(self->listeners);
	self->rHandlers = NULL;
	self->elemCounter = 0;

	//set folders
	if (ini->folders) {
		self->folders = nUtil->split(ini->folders, "|");
		for (Uint32 i = 0; i < nArray->size(self->folders); i++) {
			const char* folderId = nArray->at(self->folders, i);
			sFolder* folder = nApp->prv->getFolder(folderId);
			nUtil->assertArgument(folder);
			nArray->insert(self->folders, i, folder, NULL);
		}
	}	
	//...
	
	nApp->prv->addScene(self);
	
	return self;
}

void GxDestroyScene_(sScene* self) {

	if (self) {
		self->status = GxStatusUnloading;
		
		for(Listener* listener = nList->begin(self->listeners[nUtil->evn->ON_DESTROY]); 
			listener != NULL; listener = nList->next(self->listeners[nUtil->evn->ON_DESTROY]))
		{		
			listener->handler(&(GxEvent) {
				.target = listener->e.target,
				.type = nUtil->evn->ON_DESTROY,				
			});			
		}
		
		nGraphics->destroy(self->graphics);
		nPhysics->destroy(self->physics);
		nArray->destroy(self->elements);
		nMap->destroy(self->rHandlers);
		
		if (self->handlers && self->handlers[nUtil->evn->ON_DESTROY]){
			self->handlers[nUtil->evn->ON_DESTROY](&(GxEvent) {
				.target = self->target,
				.type = nUtil->evn->ON_DESTROY,
			});
		}	

		if(self->folders){
			for (Uint32 i = 0; i < nArray->size(self->folders); i++) {
				sFolder* folder = nArray->at(self->folders, i);
				nFolder->p->decRefCounter(folder);
			}
			nArray->destroy(self->folders);		
		}
		//events
		for (int i = 0; i < nUtil->evn->TOTAL; i++) {
			nList->destroy(self->listeners[i]);		
		}
		free(self->listeners);
		free(self->handlers);
		free(self->name);
		self->hash = 0;
		free(self);
	}
}



const char* GxSceneGetName(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->name;
}

sSize GxSceneGetSize(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->size;
}

sElement* GxSceneGetCamera(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->camera;
}

sPhysics* GxSceneGetPhysics(sScene* self) {	
	return self->physics;
}

sGraphics* GxSceneGetGraphics(sScene* self) {
	return self->graphics;
}

bool GxSceneHasStatus(sScene* self, int status) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->status == status;
}

int GxSceneGetStatus(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->status;
}

sElement* GxSceneGetElement(sScene* self, Uint32 id) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	if (id > nArray->size(self->elements)) {
		return NULL;
	}
	Uint32 l = 0;
	Uint32 r = nArray->size(self->elements);
	
	while (l <= r) { 
        Uint32 m = l + (r - l) / 2; 		
		       
		sElement* elem = nArray->at(self->elements, m);
		Uint32 elemID = nElem->p->id(elem);
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

int GxSceneGetGravity(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->gravity;
}

bool GxSceneHasGravity(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->gravity;
}

void GxSceneSetGravity(sScene* self, int gravity) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	self->gravity = gravity > 0 ? -gravity : gravity;
}

Uint32 GxSceneGetPercLoaded(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	
	if (!self->folders || !nArray->size(self->folders)) {
		return 100;
	}

	Uint32 total = 0;
	for (Uint32 i = 0; i < nArray->size(self->folders); i++){	
		sFolder* folder = nArray->at(self->folders, i);
		total += nFolder->p->getPercLoaded(folder);
	}
	
	return total / nArray->size(self->folders);
}

void GxSceneExecuteElemChildDtor_(sScene* self, void* child) {
	
	if(self->status == GxStatusUnloading){return ;}

	for(Listener* listener = nList->begin(self->listeners[nUtil->evn->ON_DESTROY]);
		listener != NULL; listener = nList->next(self->listeners[nUtil->evn->ON_DESTROY])) 
	{
		if (listener->e.target == child) {
			listener->handler(&(GxEvent) {
				.target = listener->e.target,
				.type = nUtil->evn->ON_DESTROY,				
			});
			nList->remove(self->listeners[nUtil->evn->ON_DESTROY], listener);
		}
	}
}

void GxSceneSetTimeout(sScene* self, int interval, sHandler callback, void* target) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	Timer* timer = malloc(sizeof(Timer));
	nUtil->assertAlloc(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	nList->push(self->listeners[nUtil->evn->ON_TIMEOUT], timer, free);
}

Uint32 GxSceneAddElement_(sScene* self, sElement* elem) {	
	nUtil->assertState(self->status == GxStatusLoaded || self->status == GxStatusRunning);	
	nArray->push(self->elements, elem, (sDtor) nElem->p->destroy);	
	nGraphics->insert(self->graphics, elem);
	nPhysics->insert(self->physics, elem);
	GxSceneSubscribeElemListeners_(self, elem);
	Uint32 id = self->elemCounter++;
	return id;
}


void GxSceneSubscribeElemListeners_(sScene* self, sElement* elem) {
	for (int i = 0; i < nUtil->evn->TOTAL; i++) {
		if(i == nUtil->evn->ON_DESTROY){ continue; }
		sHandler handler = nElem->getHandler(elem, i);
		if (handler) {
			void* target = nElem->target(elem);
			GxSceneAddEventListener(self, i, handler, target);
		}
	}
}

void GxSceneUnsubscribeElemListeners_(sScene* self, sElement* elem) {
	
	for (int i = 0; i < nUtil->evn->TOTAL; i++) {
		sHandler handler = nElem->getHandler(elem, i);
		if (handler) {
			void* target = nElem->target(elem);
			GxSceneRemoveEventListener(self, i, handler, target);
		}	
	}
}

void GxSceneAddEventListener(sScene* self, int type, sHandler handler, void* target) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	nUtil->assertArgument(type >= 0 && type < nUtil->evn->TOTAL);
	nUtil->assertNullPointer(handler);
	nUtil->assertNullPointer(target);
	Listener* listener = malloc(sizeof(Listener));
	nUtil->assertAlloc(listener);
	listener->e.target = target;
	listener->e.type = type;
	listener->handler = handler;
	nList->push(self->listeners[type], listener, free);
}

bool GxSceneRemoveEventListener(sScene* self, int type, sHandler handler, void* target) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	if (self->status == GxStatusUnloading) { return true; }

	nUtil->assertArgument(type >= 0 && type < nUtil->evn->TOTAL);
	nUtil->assertNullPointer(handler);
	nUtil->assertNullPointer(target);
	for(Listener* listener = nList->begin(self->listeners[type]);
		listener != NULL; listener = nList->next(self->listeners[type])) {
		if (listener->handler == handler && listener->e.target == target) {
			nList->remove(self->listeners[type], listener);
			return true;
		}
	}
	return false;
}

static inline void sceneExecuteListeners(sScene* self, int type, SDL_Event* sdle) {	
		
	if (self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent) {
			.target = self->target,
			.type = type,	
			.sdle = sdle
		});
	}
	
	for (Listener* listener = nList->begin(self->listeners[type]); listener != NULL;
		listener = nList->next(self->listeners[type]))
	{			
		listener->e.sdle = sdle;
		listener->handler(&listener->e);		
	}
}

static inline void sceneExecuteContactListeners(sScene* self, int type, sContact* contact) {
	sElement* elemSelf = nContact->colliding(contact);
	sElement* elemOther = nContact->collided(contact);
	nElem->body->p->setMcFlag(elemSelf, true);
	nElem->body->p->setMcFlag(elemOther, true);

	if (self->handlers && self->handlers[type]){
		self->handlers[type](&(GxEvent) {
			.target = self->target,
			.type = type,	
			.contact = contact
		});
	}
	nElem->p->executeContactHandler(elemSelf, type, contact);
	nElem->p->executeContactHandler(elemOther, type, contact);
	
	nElem->body->p->setMcFlag(elemSelf, false);
	nElem->body->p->setMcFlag(elemOther, false);
}

void GxScenePreLoad_(sScene* self) {
	
	nUtil->assertState(self->status == GxStatusNone);

	self->status = GxStatusLoading;
	self->elemCounter = 0;

	//initialize containers and folders
	self->graphics = nGraphics->create(self);
	self->physics = nPhysics->create(self);
	self->elements = nArray->create();			
	
	for (int i = 0; i < nUtil->evn->TOTAL; i++) {
		self->listeners[i] = nList->create();
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < nArray->size(self->folders); i++) {		
			nFolder->p->incRefCounter(nArray->at(self->folders, i));
		}
	}
}

static void GxSceneLoad_(sScene* self) {		
	
	//create physic walls and camera
	nPhysics->createWalls(self->physics);
	sSize size = nApp->logicalSize();
	self->camera = nElem->create(&(sIni) {
		.display = nElem->display->NONE,
		.body = nElem->body->FIXED,
		.className = "__CAMERA__",		
		.position = &(SDL_Rect){ 0, 0, size.w, size.h },				
	});

	nElem->body->setCmask(self->camera, GxCmaskCamera);
	
	//execute onload callbacks 
	sceneExecuteListeners(self, nUtil->evn->ON_LOAD, NULL);

	//change status to running
	self->status = GxStatusRunning;
}

void GxSceneUnload_(sScene* self) {
	self->status = GxStatusUnloading;
	nGraphics->destroy(self->graphics);
	nPhysics->destroy(self->physics);	
	//delete buttons_;
	
	sceneExecuteListeners(self, nUtil->evn->ON_UNLOAD, NULL);
	
	for(Listener* listener = nList->begin(self->listeners[nUtil->evn->ON_DESTROY]); 
		listener != NULL; listener = nList->next(self->listeners[nUtil->evn->ON_DESTROY]))
	{		
		listener->handler(&(GxEvent) {
			.target = listener->e.target,
			.type = nUtil->evn->ON_DESTROY,				
		});			
	}
	nArray->destroy(self->elements);	
	
	if(self->folders){
		for (Uint32 i = 0; i< nArray->size(self->folders); i++){
			sFolder* folder = nArray->at(self->folders, i);			
			nFolder->p->decRefCounter(folder);
		}	
	}
			
	for (int i = 0; i < nUtil->evn->TOTAL; i++) {
		nList->destroy(self->listeners[i]);
		self->listeners[i] = NULL;
	}
			
	self->graphics = NULL;
	self->physics = NULL;
	self->elements = NULL;	
	self->camera = NULL;
	
	//status		
	self->status = GxStatusNone;
}

void GxSceneRemoveElement_(sScene* self, sElement* elem) {	
	if (nElem->hasBody(elem)) nPhysics->remove(self->physics, elem);
	if (nElem->isRenderable(elem)) nGraphics->remove(self->graphics, elem);
	if (elem != self->camera) GxSceneUnsubscribeElemListeners_(self, elem);
	nArray->removeByValue(self->elements, elem);	
}

void GxSceneOnLoopBegin_(sScene* self) {	
	if(self->status == GxStatusRunning){
		sceneExecuteListeners(self, nUtil->evn->ON_LOOP_BEGIN, NULL);		
	}
}

void GxSceneOnUpdate_(sScene* self) {
	
	if (self->status == GxStatusLoading) {
		if (GxSceneGetPercLoaded(self) == 100) {
			self->status = GxStatusLoaded;
			GxSceneLoad_(self);
		}		
	}

	if(self->status == GxStatusRunning){

		for (Timer* timer = nList->begin(self->listeners[nUtil->evn->ON_TIMEOUT]); timer != NULL;
			timer = nList->next(self->listeners[nUtil->evn->ON_TIMEOUT])
		){						
			if (--timer->counter <= 0) {
				timer->callback(&(GxEvent){
					.target = timer->target,
					.type = nUtil->evn->ON_TIMEOUT
				});	
				nList->remove(self->listeners[nUtil->evn->ON_TIMEOUT], timer);
			}
		}
	
		//execute update callbacks, then update physics
		sceneExecuteListeners(self, nUtil->evn->ON_UPDATE, NULL);
		nPhysics->update(self->physics);	

		//execute preGraphical callbacks, then update graphics	
		nGraphics->update(self->graphics);		
	}
}

void GxSceneOnLoopEnd_(sScene* self) {
	if(self->status == GxStatusRunning){
		sceneExecuteListeners(self, nUtil->evn->ON_LOOP_END, NULL);
	}
}

void GxSceneOnSDLEvent_(sScene* self, SDL_Event* e) {	
	if (self->status != GxStatusRunning) {
		return;
	}

	switch (e->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:			
			sceneExecuteListeners(self, nUtil->evn->ON_KEYBOARD, e);
			break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:			
			sceneExecuteListeners(self, nUtil->evn->ON_MOUSE, e);
			break;
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_MULTIGESTURE:
		case SDL_DOLLARGESTURE:			
			sceneExecuteListeners(self, nUtil->evn->ON_FINGER, e);
			break;
		default:		
			sceneExecuteListeners(self, nUtil->evn->ON_SDL_DEFAULT, e);
			break;
	}	
}

void GxSceneOnPreContact_(sScene* self, sContact* contact) {
	sceneExecuteContactListeners(self, nUtil->evn->ON_PRE_CONTACT, contact);
}

void GxSceneOnContactBegin_(sScene* self, sContact* contact) {
	sceneExecuteContactListeners(self, nUtil->evn->ON_CONTACT_BEGIN, contact);
}

void GxSceneOnContactEnd_(sScene* self, sContact* contact) {
	sceneExecuteContactListeners(self, nUtil->evn->ON_CONTACT_END, contact);
}