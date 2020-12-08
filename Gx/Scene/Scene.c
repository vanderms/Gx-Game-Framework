#include "../Util/Util.h"
#include "../Scene/Scene.h"
#include "../App/App.h"
#include "../Element/Element.h"
#include "../Map/Map.h"
#include "../List/List.h"
#include "../Graphics/Graphics.h"
#include "../Physics/Physics.h"
#include "../Folder/Folder.h"
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
	sComponent* comp;
	sMap* components;


	//...
	Uint32 elemCounter;
} sScene;

typedef struct Timer {
	int counter;
	void* target;		
	sHandler callback;	
} Timer;


//constructor and destructor
static sScene* create(const sIni* ini) {
	
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
	self->status = nUtil->status->NONE;
	self->gravity = ini->gravity > 0 ? -ini->gravity : ini->gravity;	
		
	self->comp = nComponent->create(ini);
	if (self->comp){
		if(!self->comp->target) {
			self->comp->target = self;
		}				
	}

	self->listeners = calloc(nComponent->TOTAL, sizeof(sList*));
	nUtil->assertAlloc(self->listeners);	
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

static void pDestroy(sScene* self) {

	if (self) {
		self->status = nUtil->status->UNLOADING;
		
		if (self->components) {
			for (Uint32 i = 0; i < nMap->size(self->components); i++) {
				sComponent* comp = nMap->at(self->components, i);
				if (comp->onDestroy) {
					comp->onDestroy(&(sEvent) {
						.target = comp->target,
						.type = nComponent->ON_DESTROY
					});
				}
			}			
		}
		nMap->destroy(self->components);
		if(self->comp && self->comp->onDestroy){
			self->comp->onDestroy(&(sEvent){
				.target = self->comp->target, 
				.type = nComponent->ON_DESTROY, 
			});
		}
		nComponent->destroy(self->comp);
		nGraphics->destroy(self->graphics);
		nPhysics->destroy(self->physics);
		nArray->destroy(self->elements);		
		
		if(self->folders){
			for (Uint32 i = 0; i < nArray->size(self->folders); i++) {
				sFolder* folder = nArray->at(self->folders, i);
				nFolder->p->decRefCounter(folder);
			}
			nArray->destroy(self->folders);		
		}
		//events
		for (int i = 0; i < nComponent->TOTAL; i++) {
			nList->destroy(self->listeners[i]);		
		}
		free(self->listeners);		
		free(self->name);
		self->hash = 0;
		free(self);
	}
}



static const char* name(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->name;
}

static sSize size(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->size;
}

static sElement* getCamera(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->camera;
}

static sPhysics* pGetPhysics(sScene* self) {	
	return self->physics;
}

static sGraphics* pGetGraphics(sScene* self) {
	return self->graphics;
}

static bool hasStatus(sScene* self, int status) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->status == status;
}

static int status(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->status;
}

static sElement* getElem(sScene* self, Uint32 id) {
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

static int gravity(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->gravity;
}

static bool hasGravity(sScene* self) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	return self->gravity;
}

static void setGravity(sScene* self, int gravity) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	self->gravity = gravity > 0 ? -gravity : gravity;
}

static Uint32 getPercLoaded(sScene* self) {
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


static void setTimeout(sScene* self, int interval, sHandler callback, void* target) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	Timer* timer = malloc(sizeof(Timer));
	nUtil->assertAlloc(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	nList->push(self->listeners[nComponent->ON_TIMEOUT], timer, free);
}

static Uint32 addElem(sScene* self, sElement* elem) {	
	nUtil->assertState(self->status == nUtil->status->LOADED || self->status == nUtil->status->RUNNING);	
	nArray->push(self->elements, elem, (sDtor) nElem->p->destroy);	
	nGraphics->insert(self->graphics, elem);
	nPhysics->insert(self->physics, elem);	
	Uint32 id = self->elemCounter++;
	return id;
}

static void addComponent(sScene* self, sComponent* comp) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertArgument(comp->name && !nComponent->isComponentEmpty(comp));
	sComponent* copy = nComponent->copy(comp);
	if (!self->components) {
		self->components = nMap->create();
	}
	nMap->set(self->components, comp->name, copy, nComponent->destroy);
	nScene->p->subscribeComponent(self, comp);
}


static void pSubscribeComponent(sScene* self, sComponent* comp) {	
	
	if(comp->onLoad){
		nList->push(self->listeners[nComponent->ON_LOAD], comp, NULL);						
	}
	if (comp->onLoopBegin) {
		nList->push(self->listeners[nComponent->ON_LOOP_BEGIN], comp, NULL);
	}
	if(comp->onUpdate){
		nList->push(self->listeners[nComponent->ON_UPDATE], comp, NULL);			
	}
	if(comp->onLoopEnd){
		nList->push(self->listeners[nComponent->ON_LOOP_END], comp, NULL);			
	}
	if(comp->onUnload){
		nList->push(self->listeners[nComponent->ON_UNLOAD], comp, NULL);		
	}	
	if(comp->onKeyboard){
		nList->push(self->listeners[nComponent->ON_KEYBOARD], comp, NULL);				
	}
	if(comp->onMouse){
		nList->push(self->listeners[nComponent->ON_MOUSE], comp, NULL);			
	}
	if(comp->onFinger){
		nList->push(self->listeners[nComponent->ON_FINGER], comp, NULL);				
	}
	if(comp->onSDLDefault){
		nList->push(self->listeners[nComponent->ON_SDL_DEFAULT], comp, NULL);				
	}
}

static void pUnsubscribeComponent(sScene* self, sComponent* comp) {
	
	if (self->status == nUtil->status->UNLOADING) { 
		return; 
	}

	if(comp->onLoad){
		nList->remove(self->listeners[nComponent->ON_LOAD], comp);						
	}
	if (comp->onLoopBegin) {
		nList->remove(self->listeners[nComponent->ON_LOOP_BEGIN], comp);
	}
	if(comp->onUpdate){
		nList->remove(self->listeners[nComponent->ON_UPDATE], comp);			
	}
	if(comp->onLoopEnd){
		nList->remove(self->listeners[nComponent->ON_LOOP_END], comp);			
	}
	if(comp->onUnload){
		nList->remove(self->listeners[nComponent->ON_UNLOAD], comp);		
	}	
	if(comp->onKeyboard){
		nList->remove(self->listeners[nComponent->ON_KEYBOARD], comp);				
	}
	if(comp->onMouse){
		nList->remove(self->listeners[nComponent->ON_MOUSE], comp);			
	}
	if(comp->onFinger){
		nList->remove(self->listeners[nComponent->ON_FINGER], comp);				
	}
	if(comp->onSDLDefault){
		nList->remove(self->listeners[nComponent->ON_SDL_DEFAULT], comp);				
	}
}

static void pPreLoad(sScene* self) {
	
	nUtil->assertState(self->status == nUtil->status->NONE);

	self->status = nUtil->status->LOADING;
	self->elemCounter = 0;

	//initialize containers and folders
	self->graphics = nGraphics->create(self);
	self->physics = nPhysics->create(self);
	self->elements = nArray->create();			
	
	for (int i = 0; i < nComponent->TOTAL; i++) {
		self->listeners[i] = nList->create();
	}
	
	if(self->comp) {
		nScene->p->subscribeComponent(self, self->comp);
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < nArray->size(self->folders); i++) {		
			nFolder->p->incRefCounter(nArray->at(self->folders, i));
		}
	}
}

static void pLoad(sScene* self) {		
	
	//create physic walls and camera
	nPhysics->createWalls(self->physics);
	sSize size = nApp->logicalSize();
	self->camera = nElem->create(&(sIni) {
		.display = nElem->display->NONE,
		.body = nElem->body->FIXED,
		.className = "__CAMERA__",		
		.position = &(sRect){ 0, 0, size.w, size.h },				
	});

	nElem->body->setCmask(self->camera, nElem->body->CMASK_CAMERA);
	
	//execute onload callbacks 
	for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_LOAD]); 
		comp != NULL; comp = nList->next(self->listeners[nComponent->ON_LOAD])
	){
		nUtil->assertState(comp->onLoad);
		comp->onLoad(&(sEvent){
			.target = comp->target,
			.type = nComponent->ON_LOAD
		});
	}

	//change status to running
	self->status = nUtil->status->RUNNING;
}

static void pUnLoad(sScene* self) {
	
	self->status = nUtil->status->UNLOADING;	
	
	//execute unload callbacks	
	for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_UNLOAD]); 
		comp != NULL; comp = nList->next(self->listeners[nComponent->ON_UNLOAD])
	){
		nUtil->assertState(comp->onUnload);
		comp->onUnload(&(sEvent){
			.target = comp->target,
			.type = nComponent->ON_UNLOAD
		});
	}
	
	//destroy elements
	nArray->destroy(self->elements);
	nGraphics->destroy(self->graphics);
	nPhysics->destroy(self->physics);	
	
	if(self->folders){
		for (Uint32 i = 0; i< nArray->size(self->folders); i++){
			sFolder* folder = nArray->at(self->folders, i);			
			nFolder->p->decRefCounter(folder);
		}	
	}
	
	//destroy self->listeners lists
	for (int i = 0; i < nComponent->TOTAL; i++) {
		nList->destroy(self->listeners[i]);
		self->listeners[i] = NULL;
	}
			
	self->graphics = NULL;
	self->physics = NULL;
	self->elements = NULL;	
	self->camera = NULL;
	
	//status		
	self->status = nUtil->status->NONE;
}

static void pRemoveElem(sScene* self, sElement* elem) {	
	if (nElem->hasBody(elem)){
		nPhysics->remove(self->physics, elem);
	}
	if (nElem->isRenderable(elem)){
		nGraphics->remove(self->graphics, elem);
	}
	nArray->removeByValue(self->elements, elem);	
}

static void pOnLoopBegin(sScene* self) {	
		
	if(self->status == nUtil->status->RUNNING){
		
		nPhysics->update(self->physics);

		for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_LOOP_BEGIN]); 
			comp != NULL; comp = nList->next(self->listeners[nComponent->ON_LOOP_BEGIN])
		){
			nUtil->assertState(comp->onLoopBegin);
			comp->onLoopBegin(&(sEvent){
				.target = comp->target,
				.type = nComponent->ON_LOOP_BEGIN
			});
		}		
	}
}

static void pUpdate(sScene* self) {
	
	if (self->status == nUtil->status->LOADING) {
		if (nScene->getPercLoaded(self) == 100) {
			self->status = nUtil->status->LOADED;
			pLoad(self);
		}		
	}

	if(self->status == nUtil->status->RUNNING){

		for (Timer* timer = nList->begin(self->listeners[nComponent->ON_TIMEOUT]); timer != NULL;
			timer = nList->next(self->listeners[nComponent->ON_TIMEOUT])
		){						
			if (--timer->counter <= 0) {
				timer->callback(&(sEvent){
					.target = timer->target,
					.type = nComponent->ON_TIMEOUT
				});	
				nList->remove(self->listeners[nComponent->ON_TIMEOUT], timer);
			}
		}
	
		//execute update callbacks, then update physics
		for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_UPDATE]); 
			comp != NULL; comp = nList->next(self->listeners[nComponent->ON_UPDATE])
		){
			nUtil->assertState(comp->onUpdate);
			comp->onUpdate(&(sEvent){
				.target = comp->target,
				.type = nComponent->ON_UPDATE
			});
		}			
		nGraphics->update(self->graphics);		
	}
}

static void pOnLoopEnd(sScene* self) {
	if(self->status == nUtil->status->RUNNING){
		for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_LOOP_END]); 
			comp != NULL; comp = nList->next(self->listeners[nComponent->ON_LOOP_END])
		){
			nUtil->assertState(comp->onLoopEnd);
			comp->onLoopEnd(&(sEvent){
				.target = comp->target,
				.type = nComponent->ON_LOOP_END
			});
		}
	}
}

static void pOnSDLEvent(sScene* self, SDL_Event* e) {	
	if (self->status != nUtil->status->RUNNING) {
		return;
	}

	switch (e->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:			
			for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_KEYBOARD]); 
				comp != NULL; comp = nList->next(self->listeners[nComponent->ON_KEYBOARD])
			){
				nUtil->assertState(comp->onKeyboard);
				comp->onKeyboard(&(sEvent){
					.target = comp->target,
					.type = nComponent->ON_KEYBOARD,
					.sdle = e
				});
			}
			break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:			
			for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_MOUSE]); 
				comp != NULL; comp = nList->next(self->listeners[nComponent->ON_MOUSE])
			){
				nUtil->assertState(comp->onMouse);
				comp->onMouse(&(sEvent){
					.target = comp->target,
					.type = nComponent->ON_MOUSE,
					.sdle = e,
				});
			}
			break;
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_MULTIGESTURE:
		case SDL_DOLLARGESTURE:			
			for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_FINGER]); 
				comp != NULL; comp = nList->next(self->listeners[nComponent->ON_FINGER])
			){
				nUtil->assertState(comp->onFinger);
				comp->onFinger(&(sEvent){
					.target = comp->target,
					.type = nComponent->ON_FINGER,
					.sdle = e,
				});
			}
			break;
		default:		
			for (sComponent* comp = nList->begin(self->listeners[nComponent->ON_SDL_DEFAULT]); 
				comp != NULL; comp = nList->next(self->listeners[nComponent->ON_SDL_DEFAULT])
			){
				nUtil->assertState(comp->onSDLDefault);
				comp->onSDLDefault(&(sEvent){
					.target = comp->target,
					.type = nComponent->ON_SDL_DEFAULT
				});
			}
			break;
	}	
}

static void pOnPreContact(sScene* self, sContact* contact) {
	sElement* elemSelf = nContact->colliding(contact);
	sElement* elemOther = nContact->collided(contact);
	nElem->body->p->setMcFlag(elemSelf, true);
	nElem->body->p->setMcFlag(elemOther, true);
	

	nElem->body->p->setMcFlag(elemSelf, false);
	nElem->body->p->setMcFlag(elemOther, false);
}

static void pOnContactBegin(sScene* self, sContact* contact) {
	sElement* elemSelf = nContact->colliding(contact);
	sElement* elemOther = nContact->collided(contact);
	nElem->body->p->setMcFlag(elemSelf, true);
	nElem->body->p->setMcFlag(elemOther, true);
	
	if (self->comp && self->comp->onPreContact) {
		self->comp->onPreContact(&(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
		});
	}
	nElem->p->executeHandler(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
	});
	nElem->p->executeHandler(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
	});
	nElem->body->p->setMcFlag(elemSelf, false);
	nElem->body->p->setMcFlag(elemOther, false);
}

static void pOnContactEnd(sScene* self, sContact* contact) {
	sElement* elemSelf = nContact->colliding(contact);
	sElement* elemOther = nContact->collided(contact);
	nElem->body->p->setMcFlag(elemSelf, true);
	nElem->body->p->setMcFlag(elemOther, true);
	
	if (self->comp && self->comp->onContactEnd) {
		self->comp->onContactEnd(&(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
		});
	}
	nElem->p->executeHandler(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
	});
	nElem->p->executeHandler(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
	});

	nElem->body->p->setMcFlag(elemSelf, false);
	nElem->body->p->setMcFlag(elemOther, false);
}


const struct sSceneNamespace* const nScene = &(struct sSceneNamespace) {
	
	.create = create,
	.getPercLoaded = getPercLoaded,
	.size = size,
	.hasStatus = hasStatus,
	.status = status,
	.getElem = getElem,
	.gravity = gravity,
	.hasGravity = hasGravity,
	.name = name,
	.setGravity = setGravity,
	.setTimeout = setTimeout,
	.getCamera = getCamera,
	.addComponent = addComponent,
	

	.p = &(struct sScenePrivateNamespace) {
		.destroy = pDestroy,
		
		.getPhysics = pGetPhysics,
		.getGraphics = pGetGraphics,	
		.addElem = addElem,
		.removeElem = pRemoveElem,
		.subscribeComponent = pSubscribeComponent,
		.unsubscribeComponent = pUnsubscribeComponent,
		.preLoad = pPreLoad,
		.unLoad = pUnLoad,
		.update = pUpdate,
		.onLoopBegin = pOnLoopBegin,
		.onLoopEnd = pOnLoopEnd,
		.onSDLEvent = pOnSDLEvent,
		.onPreContact = pOnPreContact,
		.onContactBegin = pOnContactBegin,
		.onContactEnd = pOnContactEnd,
	},
};