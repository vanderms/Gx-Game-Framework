#include "../Utilities/Util.h"
#include "../Physics/GxPhysics.h"
#include "../Quadtree/GxQuadtree.h"
#include "../Scene/GxScene.h"
#include "../Element/GxElement.h"
#include "../RigidBody/GxRigidBody.h"
#include "../Map/GxMap.h"
#include "../Array/Array.h"
#include "../List/GxList.h"
#include <string.h>
#include "../App/App.h"




typedef struct GxPhysics {
	GxScene* scene;	
	sArray* contacts;
	GxQtree* fixed;
	GxQtree* dynamic;	

	//buffers
	SDL_Rect* walls;
	sArray* emdstack;
	sArray* mvstack;
	sArray* cntend;
	int depth;
} GxPhysics;

typedef struct GxContact {
	Uint32 hash;
	GxElement* colliding;
	GxElement* collided;
	Uint32 direction;	
	int amove;
	bool effective;
	bool prevented;
} GxContact;

//... Prototypes
static inline void destroyContact(GxContact* self);
static void physicsCheckGround(GxElement * other);

//... Constructor and destructor
GxPhysics* GxCreatePhysics_(GxScene* scene) {
	GxPhysics* self = malloc(sizeof(GxPhysics));
	nsUtil->assertAlloc(self);
	self->scene = scene;	
	self->contacts = nsArr->create();
	GxSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w + 2 : size.h + 2;		
	self->dynamic = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "dynamic");
	self->fixed = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "fixed");	

	//buffers
	self->walls = NULL;
	self->emdstack = nsArr->create();
	self->mvstack = nsArr->create();
	self->cntend = nsArr->create();
	self->depth = 0;
	return self;
}

void GxDestroyPhysics_(GxPhysics* self) {		
	if (self) {		
		for (Uint32 i = 0; i < nsArr->size(self->contacts); i++){
			GxContact* contact = nsArr->at(self->contacts, i);			
			destroyContact(contact);
		}
		nsArr->destroy(self->contacts);
		nsArr->destroy(self->emdstack);
		nsArr->destroy(self->mvstack);
		nsArr->destroy(self->cntend);
		GxDestroyQtree_(self->dynamic);
		GxDestroyQtree_(self->fixed);
		free(self);
	}
}

typedef struct EmData {
	GxElement* self;
	SDL_Rect trajetory;
	SDL_Rect requestedPos;
	SDL_Rect previousPos;
	sArray* contacts;
}EmData;

static inline EmData* createEmData(GxElement* elem) {
	EmData* self = malloc(sizeof(EmData));
	nsUtil->assertAlloc(self);
	self->self = elem;
	self->requestedPos = self->previousPos = *GxElemGetPosition(elem);	
	self->requestedPos.x = self->previousPos.x + GxElemGetVelx(elem);
	self->requestedPos.y = self->previousPos.y + GxElemGetVely(elem);
	SDL_UnionRect(&self->previousPos, &self->requestedPos, &self->trajetory);
	self->contacts = NULL;
	return self;
}

static inline void destroyEmData(EmData* self) {
	if (self) {
		nsArr->destroy(self->contacts);
		free(self);
	}
}

static inline GxContact* createContact(GxElement* self, GxElement* other, int amove, Uint32 direction) {
	GxContact* contact = malloc(sizeof(GxContact));	
	nsUtil->assertAlloc(contact);
	contact->hash = nsUtil->hash->CONTACT;
	contact->colliding = self;
	contact->collided = other;
	contact->amove = amove;	
	contact->direction = direction;
	contact->prevented = false;
	contact->effective = false;
	return contact;
}

static inline void destroyContact(GxContact* self) {
	if (self) {
		GxScene* scene = GxElemGetScene(self->colliding);
		if (self->effective && !GxSceneHasStatus(scene, GxStatusUnloading)) {			
			elemRemoveContact_(self->colliding, self);
			elemRemoveContact_(self->collided, self);
			nsArr->removeByValue(GxSceneGetPhysics(scene)->contacts, self);
		}
		free(self);
	}
}

//... PHYSIC STATIC METHODS PROTOTYPES
static inline GxVector physicsProcessMovementData(GxPhysics * self);
static inline void physicsApplyGravity(GxPhysics * self, GxElement * elem);
static inline void physicsCheckContactEnd(GxPhysics* self, GxElement* element);
static inline void physicsMoveElement_(GxElement* element);
static inline bool physicsAddContact(GxPhysics* self, GxContact* contact);
static inline void physicsCheckCollision(GxElement* other);
static inline bool contactIsEqual(GxContact* lhs, GxContact* rhs);

//... METHODS
void GxPhysicsUpdate_(GxPhysics* self) {
	
	SDL_Rect area = *GxElemGetPosition(GxSceneGetCamera(self->scene));
	area.x -= 64;
	area.y -= 64;
	area.w += 128;
	area.h += 128;
	
	GxQtreeIterate_(self->dynamic, area, physicsMoveElement_, true);
	nsArr->clean(self->mvstack);
}

void GxPhysicsInsertElement_(GxPhysics* self, GxElement* element) {
	if (!GxElemIsPhysical(element)) { return; }
	GxQtreeInsert_(self->fixed, element);
	if (GxElemHasDynamicBody(element)) {
		GxQtreeInsert_(self->dynamic, element);
	}
}

void GxPhysicsRemoveElement_(GxPhysics* self, GxElement* element) {
	if (!GxElemIsPhysical(element)) return;
	GxQtreeRemove_(self->fixed, element);
	
	if (GxElemHasDynamicBody(element)) {
		GxQtreeRemove_(self->dynamic, element);
	}

	sArray* contacts = nsArr->create();
	
	for (Uint32 i = 0; i < nsArr->size(self->contacts); i++){
		GxContact* contact = nsArr->at(self->contacts, i);
		if (contact->colliding == element || contact->collided == element) {			
			nsArr->push(contacts, contact, destroyContact);
		}
	}
	//the contacts are destroyed with the array
	nsArr->destroy(contacts);
}

void GxPhysicsUpdateElementPosition_(GxPhysics* self, GxElement* element, SDL_Rect previousPos) {	
	if(GxElemIsPhysical(element)){
		if (GxElemHasDynamicBody(element)) {
			GxQtreeUpdate_(self->dynamic, element, previousPos);
		}	
		GxQtreeUpdate_(self->fixed, element, previousPos);
	}	
}

GxVector GxPhysicsMoveCalledByElem_(GxPhysics* self, GxElement* element) {
	physicsMoveElement_(element);
	return *(GxVector*) nsArr->last(self->mvstack); 
}

static inline void physicsMoveElement_(GxElement* element) {

	nsUtil->assertState(!GxElemGetMcFlag_(element));
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(element));			
	physicsApplyGravity(physics, element);
	bool cantmove = GxElemGetMovFlag_(element) || !GxElemIsMoving(element);		
	if (cantmove) {
		GxVector* vector = malloc(sizeof(GxVector));
		nsUtil->assertAlloc(vector);
		*vector = (GxVector){ 0, 0 };
		nsArr->push(physics->mvstack, vector, free);		
		return;
	}	
	GxElemSetMovFlag_(element, true);	
	physics->depth++;
	nsArr->push(physics->cntend, element, NULL);
				
	EmData* emdata = createEmData(element);
	nsArr->push(physics->emdstack, emdata, (GxDestructor) destroyEmData);
	GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckCollision, true);
	GxVector* vec = malloc(sizeof(GxVector));
	nsUtil->assertAlloc(vec);
	
	*vec = physicsProcessMovementData(physics);
	nsArr->push(physics->mvstack, vec, free);

	if (GxSceneHasGravity(physics->scene) && GxElemGetMaxgvel(element)) {		
		GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckGround, true);
	}
	
	nsArr->remove(physics->emdstack, nsArr->size(physics->emdstack) - 1);
	GxElemSetMovFlag_(element, false);	
	physics->depth--;
	if (physics->depth == 0) {
		for (Uint32 i = 0; i < nsArr->size(physics->cntend); i++){
			GxElement* elem = nsArr->at(physics->cntend, i);
			physicsCheckContactEnd(physics, elem);			
		}
		nsArr->clean(physics->cntend);
	}
}

static inline void physicsApplyGravity(GxPhysics* self, GxElement* elem) {	

	int maxgvel = GxElemGetMaxgvel(elem);
	int gravity = GxSceneGetGravity(self->scene);
	int vely = GxElemGetVely(elem);
	if (gravity && (maxgvel < vely) && !GxElemIsOnGround(elem)) {
		double acceleration = gravity / 60.0;
		GxElemAccelerate(elem, 0, acceleration);
	}	
}

static inline void physicsApplyFriction(GxPhysics* self, GxElement* element, GxVector move) {	
	if (GxSceneHasGravity(self->scene) && GxElemHasFriction(element)) {
		move = (GxVector) { move.x, move.y < 0 ? move.y : 0 };
		sArray* up = GxElemGetContacts(element, GxContactUp);
		for (Uint32 i = 0; i < nsArr->size(up); i++){
		GxContact* contact = nsArr->at(up, i); 
			GxElement* other = contact->colliding == element ? contact->collided : contact->colliding;
			GxElemMove(other, move, false);
		}
	}
}

static inline void physicsCheckCollision(GxElement* other) {	
	
	//create alias	
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(other));
	EmData* emdata = nsArr->last(physics->emdstack);
	GxElement* self = emdata->self;	

	if (self == other) return;
	if (!(GxElemGetCmask(self) & GxElemGetCmask(other))) return;

	//create alias
	GxVector v = GxElemGetVelocity(self);
	const SDL_Rect* s = GxElemGetPosition(self);
	const SDL_Rect* o = GxElemGetPosition(other);	
	
	if (SDL_HasIntersection(&emdata->trajetory, o)) {

		if (!emdata->contacts) emdata->contacts = nsArr->create();

		//check horizontal direction
		if (v.x > 0) {
			int amove = o->x - (s->x + s->w);
			if (amove >= 0 && amove < v.x) {
				nsArr->push(emdata->contacts, createContact(self, other, amove, GxContactRight), NULL);
			}
		}
		else if (v.x < 0) {
			int amove = (o->x + o->w) - s->x;
			if (amove <= 0 && amove > v.x) {
				nsArr->push(emdata->contacts, createContact(self, other, amove, GxContactLeft), NULL);
			}		
		}

		//check vertical direction
		if (v.y > 0) {
			int amove = o->y - (s->y + s->h);
			if (amove >= 0 && amove < v.y) {
				nsArr->push(emdata->contacts, createContact(self, other, amove, GxContactUp), NULL);
			}
		}
		else if (v.y < 0) {
			int amove = (o->y + o->h) - s->y;
			if (amove <= 0 && amove > v.y) {
				nsArr->push(emdata->contacts, createContact(self, other, amove, GxContactDown), NULL);
			}		
		}		
	}
}

static inline GxVector physicsProcessMovementData(GxPhysics* self) {	

	//get emdata
	EmData* emdata = nsArr->last(self->emdstack);

	//gets self velocity
	GxVector move = GxElemGetVelocity(emdata->self);

	//if there is no contact, just move the element
	if (!emdata->contacts) {			
		GxElemExecuteMove_(emdata->self, move);
		physicsApplyFriction(self, emdata->self, move);
		return move;
	}
	
	bool horizontalCollision = false, verticalCollision = false;

	for (Uint32 i = 0; i < nsArr->size(emdata->contacts); i++){
		GxContact* contact = nsArr->at(emdata->contacts, i);
		GxSceneOnPreContact_(self->scene, contact);
		if (contact->prevented) { continue; }

		int spref = GxElemGetPreference(contact->colliding);
		int opref = GxElemGetPreference(contact->collided);
		SDL_Rect spos = *GxElemGetPosition(contact->colliding);
		SDL_Rect opos = *GxElemGetPosition(contact->collided);
		
		if (contact->direction == GxContactRight && move.x >= contact->amove) {			
			if (spref > opref) {
				GxElemSetPreference(contact->collided, spref);
				GxElemMove(contact->collided, (GxVector) { (spos.x + move.x + spos.w) - opos.x, 0 }, false);				
				GxElemSetPreference(contact->collided, opref);
				contact->amove = (opos.x - (spos.x + spos.w));
			}			
			if(move.x >= contact->amove) {
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == GxContactLeft && move.x <= contact->amove) {
			if (spref > opref) {
				GxElemSetPreference(contact->collided, spref);
				GxElemMove(contact->collided, (GxVector) {(spos.x + move.x) - (opos.x + opos.w), 0 }, false);				
				GxElemSetPreference(contact->collided, opref);
				contact->amove = ((opos.x + opos.w) - spos.x);
			}			
			if(move.x <= contact->amove){
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == GxContactUp && move.y >= contact->amove) {
			if (spref > opref) {				
				GxElemSetPreference(contact->collided, spref);
				GxElemMove(contact->collided, (GxVector) { 0, (spos.y + move.y + spos.h) - opos.y}, false);				
				GxElemSetPreference(contact->collided, opref);
				contact->amove = opos.y - (spos.y + spos.h);			
			}			
			if(move.y >= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
		else if (contact->direction == GxContactDown && move.y <= contact->amove) {
			if (spref > opref) {	
				GxElemSetPreference(contact->collided, spref);
				GxElemMove(contact->collided, (GxVector) {0, (spos.y + move.y) - (opos.y + opos.h)}, false);				
				GxElemSetPreference(contact->collided, opref);
				contact->amove = (opos.y + opos.h) - spos.y;
			}			
			if(move.y <= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
	}

	//move entity to new position	
	GxElemExecuteMove_(emdata->self, move);
	physicsApplyFriction(self, emdata->self, move);

	//now check effective collisions and add to self collision array
	Uint32 previousSize = nsArr->size(self->contacts);

	double xres = 0.0, yres = 0.0; //restitution in direction x and y
	bool changeVelocity = false;

	for (Uint32 i = 0; i < nsArr->size(emdata->contacts); i++){
		GxContact* contact = nsArr->at(emdata->contacts, i); 		
		int spref = GxElemGetPreference(contact->colliding);
		int opref = GxElemGetPreference(contact->collided);
		SDL_Rect spos = *GxElemGetPosition(contact->colliding);
		SDL_Rect opos = *GxElemGetPosition(contact->collided);

		if (opref >= spref) changeVelocity = true;

		if ((contact->prevented) && 
			SDL_HasIntersection(&spos, &opos)) {			
			physicsAddContact(self, contact);			
		}
		else if ((contact->direction == GxContactRight || contact->direction == GxContactLeft) && (move.x == contact->amove)) {
			if (xres < GxElemGetRestitution(contact->collided))
				xres = GxElemGetRestitution(contact->collided);
			physicsAddContact(self, contact);	
		}
		else if ((contact->direction == GxContactUp || contact->direction == GxContactDown) && (move.y == contact->amove)) {
			if (yres < GxElemGetRestitution(contact->collided))
				yres = GxElemGetRestitution(contact->collided);
			physicsAddContact(self, contact);
		}
		else {
			destroyContact(contact);
		}
	}
		
	if (changeVelocity) {		
		if (horizontalCollision) {
			GxElemApplyHozElasticity_(emdata->self, xres);			
		}
		if (verticalCollision) {
			GxElemApplyVetElasticity_(emdata->self, yres);
		}		
	}

		//notify collision callback handler
	while (previousSize < nsArr->size(self->contacts)) {
		GxContact* contact = nsArr->at(self->contacts, previousSize);
		GxSceneOnContactBegin_(self->scene, contact);
		previousSize++;
	}
	return move;
}

bool physicsAddContact(GxPhysics* self, GxContact* contact) {	
	
	bool contains = false;

	for (Uint32 i = 0; i < nsArr->size(self->contacts); i++){
		GxContact* buffer = nsArr->at(self->contacts, i);		
		if (contactIsEqual(contact, buffer)) {
			contains = true;
			break;
		}
	}

	if (!contains) {		
		elemAddContact_(contact->colliding, contact);
		elemAddContact_(contact->collided, contact);		
		contact->effective = true;
		nsArr->push(self->contacts, contact, NULL);			
		return true;
	}

	destroyContact(contact);
	return false;	
}

void physicsCheckContactEnd(GxPhysics* self, GxElement* element) {
		
	sArray* contactsToRemove = nsArr->create();
	GxList* allContacts = GxElemGetContactList_(element);
	
	for (GxContact* contact = GxListBegin(allContacts); contact != NULL;
		contact = GxListNext(allContacts))
	{	
		SDL_Rect spos = *GxElemGetPosition(contact->colliding);
		SDL_Rect opos = *GxElemGetPosition(contact->collided);
				
		if (contact->prevented){			
			if (!SDL_HasIntersection(&spos, &opos)) {
				nsArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
			}
		}
		else if((bool) (contact->direction == GxContactRight || contact->direction == GxContactLeft)){
			bool notInTheSameRow = (spos.y >= opos.y + opos.h || spos.y + spos.h <= opos.y);
			bool touchingOnXAxis = (contact->direction ==  GxContactRight) ?
				spos.x + spos.w == opos.x : spos.x == opos.x + opos.w;
			if (notInTheSameRow || !touchingOnXAxis) {
				nsArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
			}			
		}
		else if((bool) (contact->direction == GxContactUp || contact->direction == GxContactDown)){
			/*
			Obs(22/10/2020): This code is from the time the library was written in C++.
			And now, much time later, I have no idea what this code did and why it is now commented.
			
			bool ignore_friction = element->body_->friction_ && scene_->has_gravity() &&
				(element == contact->colliding ? contact->direction == U::K::Up : contact->direction == U::K::Down);
			if (ignore_friction) {
				continue;
			}
			*/
			bool notInTheSameColumn = (spos.x >= opos.x + opos.w || spos.x + spos.w <= opos.x);
			bool touchingOnYAxis = (contact->direction == GxContactUp) ?
				spos.y + spos.h == opos.y : spos.y == opos.y + opos.h;
			if (notInTheSameColumn || !touchingOnYAxis) {
				nsArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
			}
		}
	}
	
	for (Uint32 i = 0; i < nsArr->size(contactsToRemove); i++){
		GxContact* contact = nsArr->at(contactsToRemove, i);		
		GxSceneOnContactEnd_(self->scene, contact);		
	}
	
	//... sArray destructor delete contacts //-> and GxContact GxDestructor remove it from arrays.
	nsArr->destroy(contactsToRemove);
}

static void physicsCheckGround(GxElement* other) {
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(other));
	EmData* emdata = nsArr->last(physics->emdstack);
	GxElement* self = emdata->self;
	
	const SDL_Rect* s = GxElemGetPosition(self);
	const SDL_Rect* o = GxElemGetPosition(other);

	bool samecolumn =  !(s->x >= o->x + o->w || s->x + s->w <= o->x);
	bool ytouching =  (s->y == o->y + o->h);
	if (samecolumn && ytouching) {
		GxContact* contact = createContact(self, other, 0, GxContactDown);
		GxSceneOnPreContact_(physics->scene, contact);
		if (physicsAddContact(physics, contact)) {
			GxSceneOnContactBegin_(physics->scene, contact);
		}
	}
}

void GxPhysicsCreateWalls_(GxPhysics* self) {
	
	GxSize size = GxSceneGetSize(self->scene);
	GxIni ini = {
		.className = "__WALL__",				
		.display = GxElemNone,
		.body = GxElemFixed,			
	};
	
	ini.position = &(SDL_Rect) { -1, -1, size.w + 2, 1 };
	GxElemSetCmask(GxCreateElement(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) {size.w, -1, 1, size.h + 2 };
	GxElemSetCmask(GxCreateElement(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) { -1, size.h, size.w + 2, 1 };
	GxElemSetCmask(GxCreateElement(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) { -1, -1, 1, size.h + 2  };
	GxElemSetCmask(GxCreateElement(&ini), GxCmaskAll);
}

#define CHECK_CONTACT_HASH(contact)\
{\
	uint32_t hash = *(uint32_t *) contact;\
	nsUtil->assertHash(hash == nsUtil->hash->CONTACT);\
}

GxElement* GxContactGetColliding(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->colliding;
}

GxElement* GxContactGetCollided(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->collided;
}

bool GxContactIsBetween(GxContact* contact, GxElement* self, GxElement* other) {
	CHECK_CONTACT_HASH(contact)
	bool s = self ? contact->colliding == self : true;
	bool o = other ? contact->collided == other : true;
	return s && o;
}

bool GxContactHasElement(GxContact* contact, GxElement* element) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == element || contact->collided == element);
}


GxElement* GxContactGetOppositeElement(GxContact* contact, GxElement* self) {
	CHECK_CONTACT_HASH(contact)
	nsUtil->assertArgument(contact->colliding == self || contact->collided == self);
	return (contact->colliding == self ? contact->collided : contact->colliding);
}

bool GxContactHasDirection(GxContact* contact, Uint32 direction) {
	CHECK_CONTACT_HASH(contact)
	return contact->direction & direction;
}

Uint32 GxContactGetDirection(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->direction;
}

bool GxContactIsPrevented(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->prevented;
}

void GxContactAllowCollision(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	contact->prevented = true;
}

bool GxContactIsElemRightContact(GxContact* contact, GxElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactRight) ||
		(contact->collided == self && contact->direction == GxContactLeft);
}

bool GxContactIsElemLeftContact(GxContact* contact, GxElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactLeft) ||
		(contact->collided == self && contact->direction == GxContactRight);
}

bool GxContactIsElemDownContact(GxContact* contact, GxElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactDown) ||
		(contact->collided == self && contact->direction == GxContactUp);
}

bool GxContactIsElemUpContact(GxContact* contact, GxElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactUp) ||
		(contact->collided == self && contact->direction == GxContactDown);
}

void GxContactOneWayPlatform(GxEvent* e) {

	if(!GxContactHasDirection(e->contact, GxContactDown)){
		GxContactAllowCollision(e->contact);
	}
}

static inline bool contactIsEqual(GxContact* lhs, GxContact* rhs) {
	return (
		lhs->colliding == rhs->colliding && 
		lhs->collided == rhs->collided && 
		lhs->direction == rhs->direction
	);
}
