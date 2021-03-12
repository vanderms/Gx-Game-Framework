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
sScene* nSceneCreate(const sIni* ini) {
	
	nUtilAssertState(nAppIsCreated());

	sScene* self = calloc(1, sizeof(sScene));	
	nUtilAssertAlloc(self);
	self->hash = nUtil_HASH_SCENE;

	//set id
	if (ini->name){ 
		self->name = nUtilCreateString(ini->name); 
	}	
	else {
		static unsigned long int unique = 0;
		char buffer[64];
		snprintf(buffer, 64, "AUTO::SCENE::%lu", unique++);
		self->name = nUtilCreateString(buffer);
	}
	
	sSize windowSize = nAppLogicalSize();
	self->size.w = ini->size.w > windowSize.w ? ini->size.w : windowSize.w;
	self->size.h = ini->size.h > windowSize.h ? ini->size.h : windowSize.h;
	self->status = nUTIL_STATUS_NONE;
	self->gravity = ini->gravity > 0 ? -ini->gravity : ini->gravity;	
		
	self->comp = nComponentCreate_(ini);
	if (self->comp){
		if(!self->comp->target) {
			self->comp->target = self;
		}				
	}

	self->listeners = calloc(nEVENT_TOTAL, sizeof(sList*));
	nUtilAssertAlloc(self->listeners);	
	self->elemCounter = 0;

	//set folders
	if (ini->folders) {
		self->folders = nUtilSplit(ini->folders, "|");
		for (Uint32 i = 0; i < nArraySize(self->folders); i++) {
			const char* folderId = nArrayAt(self->folders, i);
			sFolder* folder = nAppGetFolder(folderId);
			nUtilAssertArgument(folder);
			nArrayInsert(self->folders, i, folder, NULL);
		}
	}	
	//...
	
	nAppAddScene_(self);
	
	return self;
}

void nSceneDestroy_(sScene* self) {

	if (self) {
		self->status = nUTIL_STATUS_UNLOADING;
		
		if (self->components) {
			for (Uint32 i = 0; i < nMapSize(self->components); i++) {
				sComponent* comp = nMapAt(self->components, i);
				if (comp->onDestroy) {
					comp->onDestroy(&(sEvent) {
						.target = comp->target,
						.type = nEVENT_ON_DESTROY
					});
				}
			}			
		}
		nMapDestroy(self->components);
		if(self->comp && self->comp->onDestroy){
			self->comp->onDestroy(&(sEvent){
				.target = self->comp->target, 
				.type = nEVENT_ON_DESTROY, 
			});
		}
		nComponentDestroy_(self->comp);
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
		for (int i = 0; i < nEVENT_TOTAL; i++) {
			nListDestroy(self->listeners[i]);		
		}
		free(self->listeners);		
		free(self->name);
		self->hash = 0;
		free(self);
	}
}



const char* nSceneName(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->name;
}

sSize nSceneSize(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->size;
}

sElement* nSceneGetCamera(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->camera;
}

sPhysics* nSceneGetPhysics_(sScene* self) {	
	return self->physics;
}

sGraphics* nSceneGetGraphics_(sScene* self) {
	return self->graphics;
}

bool nSceneHasStatus(sScene* self, int status) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->status == status;
}

int nSceneStatus(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->status;
}

sElement* nSceneGetElem(sScene* self, Uint32 id) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
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

int nSceneGravity(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->gravity;
}

bool nSceneHasGravity(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	return self->gravity;
}

void nSceneSetGravity(sScene* self, int gravity) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	self->gravity = gravity > 0 ? -gravity : gravity;
}

Uint32 nSceneGetPercLoaded(sScene* self) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	
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


void nSceneSetTimeout(sScene* self, int interval, sHandler callback, void* target) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash((*(Uint32*) self) == nUtil_HASH_SCENE);
	Timer* timer = malloc(sizeof(Timer));
	nUtilAssertAlloc(timer);
	timer->callback = callback;
	timer->counter = interval;
	timer->target = target;	
	nListPush(self->listeners[nEVENT_ON_TIMEOUT], timer, free);
}

Uint32 nSceneAddElem_(sScene* self, sElement* elem) {	
	nUtilAssertState(self->status == nUTIL_STATUS_LOADED || self->status == nUTIL_STATUS_RUNNING);	
	nArrayPush(self->elements, elem, (sDtor) nElemDestroy_);	
	nGraphicsInsert_(self->graphics, elem);
	nPhysicsInsert_(self->physics, elem);	
	Uint32 id = self->elemCounter++;
	return id;
}

void nSceneAddComponent(sScene* self, sComponent* comp) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_SCENE);
	nUtilAssertArgument(comp->name && !nComponentIsEmpty_(comp));
	sComponent* copy = nComponentCopy_(comp);
	if (!self->components) {
		self->components = nMapCreate();
	}
	nUtilAssertArgument(nMapGet(self->components, comp->name) == NULL);
	nMapSet(self->components, comp->name, copy, nComponentDestroy_);
	nSceneSubscribeComponent_(self, copy);
}


void* nSceneGetComponent(sScene* self, const char* name) {
	nUtilAssertNullPointer(self);
	nUtilAssertHash(self->hash == nUtil_HASH_SCENE);
	if (name) {
		nUtilAssertImplementation(self->components != NULL);
		sComponent* comp = nMapGet(self->components, name);
		nUtilAssertImplementation(comp != NULL);
		return comp->target;
	}
	return (self->comp ? self->comp->target : self);	
}


void nSceneSubscribeComponent_(sScene* self, sComponent* comp) {	
	
	if(comp->onLoad){
		nListPush(self->listeners[nEVENT_ON_LOAD], comp, NULL);						
	}
	if (comp->onLoopBegin) {
		nListPush(self->listeners[nEVENT_ON_LOOP_BEGIN], comp, NULL);
	}
	if(comp->onUpdate){
		nListPush(self->listeners[nEVENT_ON_UPDATE], comp, NULL);			
	}
	if(comp->onLoopEnd){
		nListPush(self->listeners[nEVENT_ON_LOOP_END], comp, NULL);			
	}
	if(comp->onUnload){
		nListPush(self->listeners[nEVENT_ON_UNLOAD], comp, NULL);		
	}	
	if(comp->onKeyboard){
		nListPush(self->listeners[nEVENT_ON_KEYBOARD], comp, NULL);				
	}
	if(comp->onMouse){
		nListPush(self->listeners[nEVENT_ON_MOUSE], comp, NULL);			
	}
	if(comp->onFinger){
		nListPush(self->listeners[nEVENT_ON_FINGER], comp, NULL);				
	}
	if(comp->onSDLDefault){
		nListPush(self->listeners[nEVENT_ON_SDL_DEFAULT], comp, NULL);				
	}
}

void nSceneUnsubscribeComponent_(sScene* self, sComponent* comp) {
	
	if (self->status == nUTIL_STATUS_UNLOADING) { 
		return; 
	}

	if(comp->onLoad){
		nListRemove(self->listeners[nEVENT_ON_LOAD], comp);						
	}
	if (comp->onLoopBegin) {
		nListRemove(self->listeners[nEVENT_ON_LOOP_BEGIN], comp);
	}
	if(comp->onUpdate){
		nListRemove(self->listeners[nEVENT_ON_UPDATE], comp);			
	}
	if(comp->onLoopEnd){
		nListRemove(self->listeners[nEVENT_ON_LOOP_END], comp);			
	}
	if(comp->onUnload){
		nListRemove(self->listeners[nEVENT_ON_UNLOAD], comp);		
	}	
	if(comp->onKeyboard){
		nListRemove(self->listeners[nEVENT_ON_KEYBOARD], comp);				
	}
	if(comp->onMouse){
		nListRemove(self->listeners[nEVENT_ON_MOUSE], comp);			
	}
	if(comp->onFinger){
		nListRemove(self->listeners[nEVENT_ON_FINGER], comp);				
	}
	if(comp->onSDLDefault){
		nListRemove(self->listeners[nEVENT_ON_SDL_DEFAULT], comp);				
	}
}

void nScenePreLoad_(sScene* self) {
	
	nUtilAssertState(self->status == nUTIL_STATUS_NONE);

	self->status = nUTIL_STATUS_LOADING;
	self->elemCounter = 0;

	//initialize containers and folders
	self->graphics = nGraphicsCreate_(self);
	self->physics = nPhysicsCreate_(self);
	self->elements = nArrayCreate();		
	
	for (int i = 0; i < nEVENT_TOTAL; i++) {
		self->listeners[i] = nListCreate();
	}
	
	if(self->comp) {
		nSceneSubscribeComponent_(self, self->comp);
	}

	//load folders
	if(self->folders){
		for (Uint32 i = 0; i < nArraySize(self->folders); i++) {		
			nFolderIncRefCounter_(nArrayAt(self->folders, i));
		}
	}
}

static void nSceneLoad_(sScene* self) {		
	
	//create physic walls and camera
	nPhysicsCreateWalls_(self->physics);
	sSize size = nAppLogicalSize();
	self->camera = nElemCreate(&(sIni) {
		.display = nELEM_DISPLAY_NONE,
		.body = nELEM_BODY_FIXED,
		.className = "__CAMERA__",		
		.position = &(sRect){ 0, 0, size.w, size.h },			
	});

	nElemSetCmask(self->camera, nELEM_CMASK_CAMERA);
	
	//execute onload callbacks 
	for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_LOAD]); 
		comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_LOAD])
	){
		nUtilAssertState(comp->onLoad);
		comp->onLoad(&(sEvent){
			.target = comp->target,
			.type = nEVENT_ON_LOAD
		});
	}

	//change status to running
	self->status = nUTIL_STATUS_RUNNING;
}

void nSceneUnLoad_(sScene* self) {
	
	self->status = nUTIL_STATUS_UNLOADING;	
	
	//execute unload callbacks	
	for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_UNLOAD]); 
		comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_UNLOAD])
	){
		nUtilAssertState(comp->onUnload);
		comp->onUnload(&(sEvent){
			.target = comp->target,
			.type = nEVENT_ON_UNLOAD
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
	for (int i = 0; i < nEVENT_TOTAL; i++) {
		nListDestroy(self->listeners[i]);
		self->listeners[i] = NULL;
	}
			
	self->graphics = NULL;
	self->physics = NULL;
	self->elements = NULL;	
	self->camera = NULL;
	
	//status		
	self->status = nUTIL_STATUS_NONE;
}

void nSceneRemoveElem_(sScene* self, sElement* elem) {	
	if (nElemHasBody(elem)){
		nPhysicsRemoveElem_(self->physics, elem);
	}
	if (nElemIsRenderable(elem)){
		nGraphicsRemoveElement_(self->graphics, elem);
	}
	nArrayRemoveByValue(self->elements, elem);	
}

void nSceneOnLoopBegin_(sScene* self) {	
		
	if(self->status == nUTIL_STATUS_RUNNING){
		
		nPhysicsUpdate_(self->physics);

		for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_LOOP_BEGIN]); 
			comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_LOOP_BEGIN])
		){
			nUtilAssertState(comp->onLoopBegin);
			comp->onLoopBegin(&(sEvent){
				.target = comp->target,
				.type = nEVENT_ON_LOOP_BEGIN
			});
		}		
	}
}

void nSceneUpdate_(sScene* self) {
	
	if (self->status == nUTIL_STATUS_LOADING) {
		if (nSceneGetPercLoaded(self) == 100) {
			self->status = nUTIL_STATUS_LOADED;
			nSceneLoad_(self);
		}		
	}

	if(self->status == nUTIL_STATUS_RUNNING){

		for (Timer* timer = nListBegin(self->listeners[nEVENT_ON_TIMEOUT]); timer != NULL;
			timer = nListNext(self->listeners[nEVENT_ON_TIMEOUT])
		){						
			if (--timer->counter <= 0) {
				timer->callback(&(sEvent){
					.target = timer->target,
					.type = nEVENT_ON_TIMEOUT
				});	
				nListRemove(self->listeners[nEVENT_ON_TIMEOUT], timer);
			}
		}
	
		//execute update callbacks, then update physics
		for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_UPDATE]); 
			comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_UPDATE])
		){
			nUtilAssertState(comp->onUpdate);
			comp->onUpdate(&(sEvent){
				.target = comp->target,
				.type = nEVENT_ON_UPDATE
			});
		}			
		nGraphicsUpdate_(self->graphics);		
	}
}

void nSceneOnLoopEnd_(sScene* self) {
	if(self->status == nUTIL_STATUS_RUNNING){
		for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_LOOP_END]); 
			comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_LOOP_END])
		){
			nUtilAssertState(comp->onLoopEnd);
			comp->onLoopEnd(&(sEvent){
				.target = comp->target,
				.type = nEVENT_ON_LOOP_END
			});
		}
	}
}

void nSceneOnSDLEvent_(sScene* self, SDL_Event* e) {	
	if (self->status != nUTIL_STATUS_RUNNING) {
		return;
	}

	switch (e->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:			
			for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_KEYBOARD]); 
				comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_KEYBOARD])
			){
				nUtilAssertState(comp->onKeyboard);
				comp->onKeyboard(&(sEvent){
					.target = comp->target,
					.type = nEVENT_ON_KEYBOARD,
					.sdle = e
				});
			}
			break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:			
			for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_MOUSE]); 
				comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_MOUSE])
			){
				nUtilAssertState(comp->onMouse);
				comp->onMouse(&(sEvent){
					.target = comp->target,
					.type = nEVENT_ON_MOUSE,
					.sdle = e,
				});
			}
			break;
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_MULTIGESTURE:
		case SDL_DOLLARGESTURE:			
			for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_FINGER]); 
				comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_FINGER])
			){
				nUtilAssertState(comp->onFinger);
				comp->onFinger(&(sEvent){
					.target = comp->target,
					.type = nEVENT_ON_FINGER,
					.sdle = e,
				});
			}
			break;
		default:		
			for (sComponent* comp = nListBegin(self->listeners[nEVENT_ON_SDL_DEFAULT]); 
				comp != NULL; comp = nListNext(self->listeners[nEVENT_ON_SDL_DEFAULT])
			){
				nUtilAssertState(comp->onSDLDefault);
				comp->onSDLDefault(&(sEvent){
					.target = comp->target,
					.type = nEVENT_ON_SDL_DEFAULT
				});
			}
			break;
	}	
}

void nSceneOnPreContact_(sScene* self, sContact* contact) {
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	
	if (self->comp && self->comp->onPreContact) {
		self->comp->onPreContact(&(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_PRE_CONTACT,
			.contact = contact,
		});
	}
	nElemExecuteHandler_(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_PRE_CONTACT,
			.contact = contact,
	});
	nElemExecuteHandler_(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_PRE_CONTACT,
			.contact = contact,
	});

	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
}

void nSceneOnContactBegin_(sScene* self, sContact* contact) {
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	
	if (self->comp && self->comp->onContactBegin) {
		self->comp->onContactBegin(&(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_BEGIN,
			.contact = contact,
		});
	}
	nElemExecuteHandler_(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_BEGIN,
			.contact = contact,
	});
	
	nElemExecuteHandler_(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_BEGIN,
			.contact = contact,
	});
	
	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
}

void nSceneOnContactEnd_(sScene* self, sContact* contact) {
	sElement* elemSelf = nContactColliding(contact);
	sElement* elemOther = nContactCollided(contact);
	nElemSetMcFlag_(elemSelf, true);
	nElemSetMcFlag_(elemOther, true);
	
	if (self->comp && self->comp->onContactEnd) {
		self->comp->onContactEnd(&(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_END,
			.contact = contact,
		});
	}
	nElemExecuteHandler_(elemSelf, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_END,
			.contact = contact,
	});
	nElemExecuteHandler_(elemOther, &(sEvent){
			.target = self->comp->target,
			.type = nEVENT_ON_CONTACT_END,
			.contact = contact,
	});

	nElemSetMcFlag_(elemSelf, false);
	nElemSetMcFlag_(elemOther, false);
}
