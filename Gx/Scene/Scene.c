#include "../Util/Util.h"
#include "../Scene/Scene.h"
#include "../App/App.h"
#include "../Element/Element.h"
#include "../Containers/Map/Map.h"
#include "../Containers/Array/Array.h"
#include "../Containers/List/List.h"
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
	
	nUtil->assertState(nAppIsCreated());

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
	
	sSize windowSize = nAppLogicalSize();
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
		for (Uint32 i = 0; i < nArraySize(self->folders); i++) {
			const char* folderId = nArrayAt(self->folders, i);
			sFolder* folder = nAppGetFolder(folderId);
			nUtil->assertArgument(folder);
			nArrayInsert(self->folders, i, folder, NULL);
		}
	}	
	//...
	
	nAppAddScene_(self);
	
	return self;
}

static void pDestroy(sScene* self) {

	if (self) {
		self->status = nUtil->status->UNLOADING;
		
		if (self->components) {
			for (Uint32 i = 0; i < nMapSize(self->components); i++) {
				sComponent* comp = nMapAt(self->components, i);
				if (comp->onDestroy) {
					comp->onDestroy(&(sEvent) {
						.target = comp->target,
						.type = nComponent->ON_DESTROY
					});
				}
			}			
		}
		nMapDestroy(self->components);
		if(self->comp && self->comp->onDestroy){
			self->comp->onDestroy(&(sEvent){
				.target = self->comp->target, 
				.type = nComponent->ON_DESTROY, 
			});
		}
		nComponent->destroy(self->comp);
		nGraphicsDestroy_(self->graphics);
		nPhysicsDestroy_(self->physics);
		nArrayDestroy(self->elements);		
		
		if(self->folders){
			for (Uint32 i = 0; i < nArraySize(self->folders); i++) {
				sFolder* folder = nArrayAt(self->folders, i);
				nFolderDecRefCounter_(folder);
			}
			nArrayDestroy(self->folders);		
		}
		//events
		for (int i = 0; i < nComponent->TOTAL; i++) {
			nListDestroy(self->listeners[i]);		
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
	if (id > nArraySize(self->elements)) {
		return NULL;
	}
	Uint32 l = 0;
	Uint32 r = nArraySize(self->elements);
	
	while (l <= r) { 
        Uint32 m = l + (r - l) / 2; 		
		       
		sElement* elem = nArrayAt(self->elements, m);
		Uint32 elemID = nElemPId_(elem);
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
	
	if (!self->folders || !nArraySize(self->folders)) {
		return 100;
	}

	Uint32 total = 0;
	for (Uint32 i = 0; i < nArraySize(self->folders); i++){	
		sFolder* folder = nArrayAt(self->folders, i);
		total += nFolderGetPercLoaded(folder);
	}
	
	return total / nArraySize(self->folders);
}


static void setTimeout(sScene* self, int interval, sHandler callback, void* target) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash((*(Uint32*) self) == nUtil->hash->SCENE);
	Timer* timer = malloc(sizeof(Timer));
	nUtil->assertAlloc(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	nListPush(self->listeners[nComponent->ON_TIMEOUT], timer, free);
}

static Uint32 addElem(sScene* self, sElement* elem) {	
	nUtil->assertState(self->status == nUtil->status->LOADED || self->status == nUtil->status->RUNNING);	
	nArrayPush(self->elements, elem, (sDtor) nElemDestroy_);	
	nGraphicsInsert_(self->graphics, elem);
	nPhysicsInsert_(self->physics, elem);	
	Uint32 id = self->elemCounter++;
	return id;
}

static void addComponent(sScene* self, sComponent* comp) {
	nUtil->assertNullPointer(self);
	nUtil->assertHash(self->hash == nUtil->hash->ELEMENT);
	nUtil->assertArgument(comp->name && !nComponent->isComponentEmpty(comp));
	sComponent* copy = nComponent->copy(comp);
	if (!self->components) {
		self->components = nMapCreate();
	}
	nMapSet(self->components, comp->name, copy, nComponent->destroy);
	nScene->p_->subscribeComponent(self, comp);
}


static void pSubscribeComponent(sScene* self, sComponent* comp) {	
	
	if(comp->onLoad){
		nListPush(self->listeners[nComponent->ON_LOAD], comp, NULL);						
	}
	if (comp->onLoopBegin) {
		nListPush(self->listeners[nComponent->ON_LOOP_BEGIN], comp, NULL);
	}
	if(comp->onUpdate){
		nListPush(self->listeners[nComponent->ON_UPDATE], comp, NULL);			
	}
	if(comp->onLoopEnd){
		nListPush(self->listeners[nComponent->ON_LOOP_END], comp, NULL);			
	}
	if(comp->onUnload){
		nListPush(self->listeners[nComponent->ON_UNLOAD], comp, NULL);		
	}	
	if(comp->onKeyboard){
		nListPush(self->listeners[nComponent->ON_KEYBOARD], comp, NULL);				
	}
	if(comp->onMouse){
		nListPush(self->listeners[nComponent->ON_MOUSE], comp, NULL);			
	}
	if(comp->onFinger){
		nListPush(self->listeners[nComponent->ON_FINGER], comp, NULL);				
	}
	if(comp->onSDLDefault){
		nListPush(self->listeners[nComponent->ON_SDL_DEFAULT], comp, NULL);				
	}
}

static void pUnsubscribeComponent(sScene* self, sComponent* comp) {
	
	if (self->status == nUtil->status->UNLOADING) { 
		return; 
	}

	if(comp->onLoad){
		nListRemove(self->listeners[nComponent->ON_LOAD], comp);						
	}
	if (comp->onLoopBegin) {
		nListRemove(self->listeners[nComponent->ON_LOOP_BEGIN], comp);
	}
	if(comp->onUpdate){
		nListRemove(self->listeners[nComponent->ON_UPDATE], comp);			
	}
	if(comp->onLoopEnd){
		nListRemove(self->listeners[nComponent->ON_LOOP_END], comp);			
	}
	if(comp->onUnload){
		nListRemove(self->listeners[nComponent->ON_UNLOAD], comp);		
	}	
	if(comp->onKeyboard){
		nListRemove(self->listeners[nComponent->ON_KEYBOARD], comp);				
	}
	if(comp->onMouse){
		nListRemove(self->listeners[nComponent->ON_MOUSE], comp);			
	}
	if(comp->onFinger){
		nListRemove(self->listeners[nComponent->ON_FINGER], comp);				
	}
	if(comp->onSDLDefault){
		nListRemove(self->listeners[nComponent->ON_SDL_DEFAULT], comp);				
	}
}

static void pPreLoad(sScene* self) {
	
	nUtil->assertState(self->status == nUtil->status->NONE);

	self->status = nUtil->status->LOADING;
	self->elemCounter = 0;

	//initialize containers and folders
	self->graphics = nGraphicsCreate_(self);
	self->physics = nPhysicsCreate_(self);
	self->elements = nArrayCreate();		
	
	for (int i = 0; i < nComponent->TOTAL; i++) {
		self->listeners[i] = nListCreate();
	}
	
	if(self->comp) {
		nScene->p_->subscribeComponent(self, self->comp);
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < nArraySize(self->folders); i++) {		
			nFolderIncRefCounter_(nArrayAt(self->folders, i));
		}
	}
}

static void pLoad(sScene* self) {		
	
	//create physic walls and camera
	nPhysicsCreateWalls_(self->physics);
	sSize size = nAppLogicalSize();
	self->camera = nElemCreate(&(sIni) {
		.display = nElem_DISPLAY_NONE,
		.body = nElem_BODY_FIXED,
		.className = "__CAMERA__",		
		.position = &(sRect){ 0, 0, size.w, size.h },			
	});

	nElemSetCmask(self->camera, nElem_CMASK_CAMERA);
	
	//execute onload callbacks 
	for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_LOAD]); 
		comp != NULL; comp = nListNext(self->listeners[nComponent->ON_LOAD])
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
	for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_UNLOAD]); 
		comp != NULL; comp = nListNext(self->listeners[nComponent->ON_UNLOAD])
	){
		nUtil->assertState(comp->onUnload);
		comp->onUnload(&(sEvent){
			.target = comp->target,
			.type = nComponent->ON_UNLOAD
		});
	}
	
	//destroy elements
	nArrayDestroy(self->elements);
	nGraphicsDestroy_(self->graphics);
	nPhysicsDestroy_(self->physics);	
	
	if(self->folders){
		for (Uint32 i = 0; i< nArraySize(self->folders); i++){
			sFolder* folder = nArrayAt(self->folders, i);			
			nFolderDecRefCounter_(folder);
		}	
	}
	
	//destroy self->listeners lists
	for (int i = 0; i < nComponent->TOTAL; i++) {
		nListDestroy(self->listeners[i]);
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
	if (nElemHasBody(elem)){
		nPhysicsRemoveElem_(self->physics, elem);
	}
	if (nElemIsRenderable(elem)){
		nGraphicsRemoveElement_(self->graphics, elem);
	}
	nArrayRemoveByValue(self->elements, elem);	
}

static void pOnLoopBegin(sScene* self) {	
		
	if(self->status == nUtil->status->RUNNING){
		
		nPhysicsUpdate_(self->physics);

		for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_LOOP_BEGIN]); 
			comp != NULL; comp = nListNext(self->listeners[nComponent->ON_LOOP_BEGIN])
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

		for (Timer* timer = nListBegin(self->listeners[nComponent->ON_TIMEOUT]); timer != NULL;
			timer = nListNext(self->listeners[nComponent->ON_TIMEOUT])
		){						
			if (--timer->counter <= 0) {
				timer->callback(&(sEvent){
					.target = timer->target,
					.type = nComponent->ON_TIMEOUT
				});	
				nListRemove(self->listeners[nComponent->ON_TIMEOUT], timer);
			}
		}
	
		//execute update callbacks, then update physics
		for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_UPDATE]); 
			comp != NULL; comp = nListNext(self->listeners[nComponent->ON_UPDATE])
		){
			nUtil->assertState(comp->onUpdate);
			comp->onUpdate(&(sEvent){
				.target = comp->target,
				.type = nComponent->ON_UPDATE
			});
		}			
		nGraphicsUpdate_(self->graphics);		
	}
}

static void pOnLoopEnd(sScene* self) {
	if(self->status == nUtil->status->RUNNING){
		for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_LOOP_END]); 
			comp != NULL; comp = nListNext(self->listeners[nComponent->ON_LOOP_END])
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
			for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_KEYBOARD]); 
				comp != NULL; comp = nListNext(self->listeners[nComponent->ON_KEYBOARD])
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
			for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_MOUSE]); 
				comp != NULL; comp = nListNext(self->listeners[nComponent->ON_MOUSE])
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
			for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_FINGER]); 
				comp != NULL; comp = nListNext(self->listeners[nComponent->ON_FINGER])
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
			for (sComponent* comp = nListBegin(self->listeners[nComponent->ON_SDL_DEFAULT]); 
				comp != NULL; comp = nListNext(self->listeners[nComponent->ON_SDL_DEFAULT])
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
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	

	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
}

static void pOnContactBegin(sScene* self, sContact* contact) {
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	
	if (self->comp && self->comp->onPreContact) {
		self->comp->onPreContact(&(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
		});
	}
	nElemExecuteHandler_(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
	});
	nElemExecuteHandler_(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_PRE_CONTACT,
			.contact = contact,
	});
	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
}

static void pOnContactEnd(sScene* self, sContact* contact) {
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	
	if (self->comp && self->comp->onContactEnd) {
		self->comp->onContactEnd(&(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
		});
	}
	nElemExecuteHandler_(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
	});
	nElemExecuteHandler_(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nComponent->ON_CONTACT_END,
			.contact = contact,
	});

	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
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
	

	.p_ = &(struct sScenePrivateNamespace) {
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