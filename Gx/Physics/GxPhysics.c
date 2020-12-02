#include "../Utilities/Util.h"
#include "../Physics/GxPhysics.h"
#include "../Quadtree/GxQuadtree.h"
#include "../Scene/GxScene.h"
#include "../sElement/sElement.h"
#include "../RigidBody/sElemBody.h"
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
	sElement* colliding;
	sElement* collided;
	Uint32 direction;	
	int amove;
	bool effective;
	bool prevented;
} GxContact;

//... Prototypes
static inline void destroyContact(GxContact* self);
static void physicsCheckGround(sElement * other);

//... Constructor and destructor
GxPhysics* GxCreatePhysics_(GxScene* scene) {
	GxPhysics* self = malloc(sizeof(GxPhysics));
	nUtil->assertAlloc(self);
	self->scene = scene;	
	self->contacts = nArr->create();
	GxSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w + 2 : size.h + 2;		
	self->dynamic = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "dynamic");
	self->fixed = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "fixed");	

	//buffers
	self->walls = NULL;
	self->emdstack = nArr->create();
	self->mvstack = nArr->create();
	self->cntend = nArr->create();
	self->depth = 0;
	return self;
}

void GxDestroyPhysics_(GxPhysics* self) {		
	if (self) {		
		for (Uint32 i = 0; i < nArr->size(self->contacts); i++){
			GxContact* contact = nArr->at(self->contacts, i);			
			destroyContact(contact);
		}
		nArr->destroy(self->contacts);
		nArr->destroy(self->emdstack);
		nArr->destroy(self->mvstack);
		nArr->destroy(self->cntend);
		GxDestroyQtree_(self->dynamic);
		GxDestroyQtree_(self->fixed);
		free(self);
	}
}

typedef struct EmData {
	sElement* self;
	SDL_Rect trajetory;
	SDL_Rect requestedPos;
	SDL_Rect previousPos;
	sArray* contacts;
}EmData;

static inline EmData* createEmData(sElement* elem) {
	EmData* self = malloc(sizeof(EmData));
	nUtil->assertAlloc(self);
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
		nArr->destroy(self->contacts);
		free(self);
	}
}

static inline GxContact* createContact(sElement* self, sElement* other, int amove, Uint32 direction) {
	GxContact* contact = malloc(sizeof(GxContact));	
	nUtil->assertAlloc(contact);
	contact->hash = nUtil->hash->CONTACT;
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
			nArr->removeByValue(GxSceneGetPhysics(scene)->contacts, self);
		}
		free(self);
	}
}

//... PHYSIC STATIC METHODS PROTOTYPES
static inline sVector physicsProcessMovementData(GxPhysics * self);
static inline void physicsApplyGravity(GxPhysics * self, sElement * elem);
static inline void physicsCheckContactEnd(GxPhysics* self, sElement* element);
static inline void physicsMoveElement_(sElement* element);
static inline bool physicsAddContact(GxPhysics* self, GxContact* contact);
static inline void physicsCheckCollision(sElement* other);
static inline bool contactIsEqual(GxContact* lhs, GxContact* rhs);

//... METHODS
void GxPhysicsUpdate_(GxPhysics* self) {
	
	SDL_Rect area = *GxElemGetPosition(GxSceneGetCamera(self->scene));
	area.x -= 64;
	area.y -= 64;
	area.w += 128;
	area.h += 128;
	
	GxQtreeIterate_(self->dynamic, area, physicsMoveElement_, true);
	nArr->clean(self->mvstack);
}

void GxPhysicsInsertElement_(GxPhysics* self, sElement* element) {
	if (!GxElemIsPhysical(element)) { return; }
	GxQtreeInsert_(self->fixed, element);
	if (GxElemHasDynamicBody(element)) {
		GxQtreeInsert_(self->dynamic, element);
	}
}

void GxPhysicsRemoveElement_(GxPhysics* self, sElement* element) {
	if (!GxElemIsPhysical(element)) return;
	GxQtreeRemove_(self->fixed, element);
	
	if (GxElemHasDynamicBody(element)) {
		GxQtreeRemove_(self->dynamic, element);
	}

	sArray* contacts = nArr->create();
	
	for (Uint32 i = 0; i < nArr->size(self->contacts); i++){
		GxContact* contact = nArr->at(self->contacts, i);
		if (contact->colliding == element || contact->collided == element) {			
			nArr->push(contacts, contact, destroyContact);
		}
	}
	//the contacts are destroyed with the array
	nArr->destroy(contacts);
}

void GxPhysicsUpdateElementPosition_(GxPhysics* self, sElement* element, SDL_Rect previousPos) {	
	if(GxElemIsPhysical(element)){
		if (GxElemHasDynamicBody(element)) {
			GxQtreeUpdate_(self->dynamic, element, previousPos);
		}	
		GxQtreeUpdate_(self->fixed, element, previousPos);
	}	
}

sVector GxPhysicsMoveCalledByElem_(GxPhysics* self, sElement* element) {
	physicsMoveElement_(element);
	return *(sVector*) nArr->last(self->mvstack); 
}

static inline void physicsMoveElement_(sElement* element) {

	nUtil->assertState(!GxElemGetMcFlag_(element));
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(element));			
	physicsApplyGravity(physics, element);
	bool cantmove = GxElemGetMovFlag_(element) || !GxElemIsMoving(element);		
	if (cantmove) {
		sVector* vector = malloc(sizeof(sVector));
		nUtil->assertAlloc(vector);
		*vector = (sVector){ 0, 0 };
		nArr->push(physics->mvstack, vector, free);		
		return;
	}	
	GxElemSetMovFlag_(element, true);	
	physics->depth++;
	nArr->push(physics->cntend, element, NULL);
				
	EmData* emdata = createEmData(element);
	nArr->push(physics->emdstack, emdata, (GxDestructor) destroyEmData);
	GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckCollision, true);
	sVector* vec = malloc(sizeof(sVector));
	nUtil->assertAlloc(vec);
	
	*vec = physicsProcessMovementData(physics);
	nArr->push(physics->mvstack, vec, free);

	if (GxSceneHasGravity(physics->scene) && GxElemGetMaxgvel(element)) {		
		GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckGround, true);
	}
	
	nArr->remove(physics->emdstack, nArr->size(physics->emdstack) - 1);
	GxElemSetMovFlag_(element, false);	
	physics->depth--;
	if (physics->depth == 0) {
		for (Uint32 i = 0; i < nArr->size(physics->cntend); i++){
			sElement* elem = nArr->at(physics->cntend, i);
			physicsCheckContactEnd(physics, elem);			
		}
		nArr->clean(physics->cntend);
	}
}

static inline void physicsApplyGravity(GxPhysics* self, sElement* elem) {	

	int maxgvel = GxElemGetMaxgvel(elem);
	int gravity = GxSceneGetGravity(self->scene);
	int vely = GxElemGetVely(elem);
	if (gravity && (maxgvel < vely) && !GxElemIsOnGround(elem)) {
		double acceleration = gravity / 60.0;
		GxElemAccelerate(elem, 0, acceleration);
	}	
}

static inline void physicsApplyFriction(GxPhysics* self, sElement* element, sVector move) {	
	if (GxSceneHasGravity(self->scene) && GxElemHasFriction(element)) {
		move = (sVector) { move.x, move.y < 0 ? move.y : 0 };
		sArray* up = GxElemGetContacts(element, GxContactUp);
		for (Uint32 i = 0; i < nArr->size(up); i++){
		GxContact* contact = nArr->at(up, i); 
			sElement* other = contact->colliding == element ? contact->collided : contact->colliding;
			GxElemMove(other, move, false);
		}
	}
}

static inline void physicsCheckCollision(sElement* other) {	
	
	//create alias	
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(other));
	EmData* emdata = nArr->last(physics->emdstack);
	sElement* self = emdata->self;	

	if (self == other) return;
	if (!(GxElemGetCmask(self) & GxElemGetCmask(other))) return;

	//create alias
	sVector v = GxElemGetVelocity(self);
	const SDL_Rect* s = GxElemGetPosition(self);
	const SDL_Rect* o = GxElemGetPosition(other);	
	
	if (SDL_HasIntersection(&emdata->trajetory, o)) {

		if (!emdata->contacts) emdata->contacts = nArr->create();

		//check horizontal direction
		if (v.x > 0) {
			int amove = o->x - (s->x + s->w);
			if (amove >= 0 && amove < v.x) {
				nArr->push(emdata->contacts, createContact(self, other, amove, GxContactRight), NULL);
			}
		}
		else if (v.x < 0) {
			int amove = (o->x + o->w) - s->x;
			if (amove <= 0 && amove > v.x) {
				nArr->push(emdata->contacts, createContact(self, other, amove, GxContactLeft), NULL);
			}		
		}

		//check vertical direction
		if (v.y > 0) {
			int amove = o->y - (s->y + s->h);
			if (amove >= 0 && amove < v.y) {
				nArr->push(emdata->contacts, createContact(self, other, amove, GxContactUp), NULL);
			}
		}
		else if (v.y < 0) {
			int amove = (o->y + o->h) - s->y;
			if (amove <= 0 && amove > v.y) {
				nArr->push(emdata->contacts, createContact(self, other, amove, GxContactDown), NULL);
			}		
		}		
	}
}

static inline sVector physicsProcessMovementData(GxPhysics* self) {	

	//get emdata
	EmData* emdata = nArr->last(self->emdstack);

	//gets self velocity
	sVector move = GxElemGetVelocity(emdata->self);

	//if there is no contact, just move the element
	if (!emdata->contacts) {			
		GxElemExecuteMove_(emdata->self, move);
		physicsApplyFriction(self, emdata->self, move);
		return move;
	}
	
	bool horizontalCollision = false, verticalCollision = false;

	for (Uint32 i = 0; i < nArr->size(emdata->contacts); i++){
		GxContact* contact = nArr->at(emdata->contacts, i);
		GxSceneOnPreContact_(self->scene, contact);
		if (contact->prevented) { continue; }

		int spref = GxElemGetPreference(contact->colliding);
		int opref = GxElemGetPreference(contact->collided);
		SDL_Rect spos = *GxElemGetPosition(contact->colliding);
		SDL_Rect opos = *GxElemGetPosition(contact->collided);
		
		if (contact->direction == GxContactRight && move.x >= contact->amove) {			
			if (spref > opref) {
				GxElemSetPreference(contact->collided, spref);
				GxElemMove(contact->collided, (sVector) { (spos.x + move.x + spos.w) - opos.x, 0 }, false);				
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
				GxElemMove(contact->collided, (sVector) {(spos.x + move.x) - (opos.x + opos.w), 0 }, false);				
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
				GxElemMove(contact->collided, (sVector) { 0, (spos.y + move.y + spos.h) - opos.y}, false);				
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
				GxElemMove(contact->collided, (sVector) {0, (spos.y + move.y) - (opos.y + opos.h)}, false);				
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
	Uint32 previousSize = nArr->size(self->contacts);

	double xres = 0.0, yres = 0.0; //restitution in direction x and y
	bool changeVelocity = false;

	for (Uint32 i = 0; i < nArr->size(emdata->contacts); i++){
		GxContact* contact = nArr->at(emdata->contacts, i); 		
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
	while (previousSize < nArr->size(self->contacts)) {
		GxContact* contact = nArr->at(self->contacts, previousSize);
		GxSceneOnContactBegin_(self->scene, contact);
		previousSize++;
	}
	return move;
}

bool physicsAddContact(GxPhysics* self, GxContact* contact) {	
	
	bool contains = false;

	for (Uint32 i = 0; i < nArr->size(self->contacts); i++){
		GxContact* buffer = nArr->at(self->contacts, i);		
		if (contactIsEqual(contact, buffer)) {
			contains = true;
			break;
		}
	}

	if (!contains) {		
		elemAddContact_(contact->colliding, contact);
		elemAddContact_(contact->collided, contact);		
		contact->effective = true;
		nArr->push(self->contacts, contact, NULL);			
		return true;
	}

	destroyContact(contact);
	return false;	
}

void physicsCheckContactEnd(GxPhysics* self, sElement* element) {
		
	sArray* contactsToRemove = nArr->create();
	GxList* allContacts = GxElemGetContactList_(element);
	
	for (GxContact* contact = GxListBegin(allContacts); contact != NULL;
		contact = GxListNext(allContacts))
	{	
		SDL_Rect spos = *GxElemGetPosition(contact->colliding);
		SDL_Rect opos = *GxElemGetPosition(contact->collided);
				
		if (contact->prevented){			
			if (!SDL_HasIntersection(&spos, &opos)) {
				nArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
			}
		}
		else if((bool) (contact->direction == GxContactRight || contact->direction == GxContactLeft)){
			bool notInTheSameRow = (spos.y >= opos.y + opos.h || spos.y + spos.h <= opos.y);
			bool touchingOnXAxis = (contact->direction ==  GxContactRight) ?
				spos.x + spos.w == opos.x : spos.x == opos.x + opos.w;
			if (notInTheSameRow || !touchingOnXAxis) {
				nArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
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
				nArr->push(contactsToRemove, contact, (GxDestructor) destroyContact);
			}
		}
	}
	
	for (Uint32 i = 0; i < nArr->size(contactsToRemove); i++){
		GxContact* contact = nArr->at(contactsToRemove, i);		
		GxSceneOnContactEnd_(self->scene, contact);		
	}
	
	//... sArray destructor delete contacts //-> and GxContact GxDestructor remove it from arrays.
	nArr->destroy(contactsToRemove);
}

static void physicsCheckGround(sElement* other) {
	GxPhysics* physics = GxSceneGetPhysics(GxElemGetScene(other));
	EmData* emdata = nArr->last(physics->emdstack);
	sElement* self = emdata->self;
	
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
	sIni ini = {
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
	nUtil->assertHash(hash == nUtil->hash->CONTACT);\
}

sElement* GxContactGetColliding(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->colliding;
}

sElement* GxContactGetCollided(GxContact* contact) {
	CHECK_CONTACT_HASH(contact)
	return contact->collided;
}

bool GxContactIsBetween(GxContact* contact, sElement* self, sElement* other) {
	CHECK_CONTACT_HASH(contact)
	bool s = self ? contact->colliding == self : true;
	bool o = other ? contact->collided == other : true;
	return s && o;
}

bool GxContactHasElement(GxContact* contact, sElement* element) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == element || contact->collided == element);
}


sElement* GxContactGetOppositeElement(GxContact* contact, sElement* self) {
	CHECK_CONTACT_HASH(contact)
	nUtil->assertArgument(contact->colliding == self || contact->collided == self);
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

bool GxContactIsElemRightContact(GxContact* contact, sElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactRight) ||
		(contact->collided == self && contact->direction == GxContactLeft);
}

bool GxContactIsElemLeftContact(GxContact* contact, sElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactLeft) ||
		(contact->collided == self && contact->direction == GxContactRight);
}

bool GxContactIsElemDownContact(GxContact* contact, sElement* self) {
	CHECK_CONTACT_HASH(contact)
	return (contact->colliding == self && contact->direction == GxContactDown) ||
		(contact->collided == self && contact->direction == GxContactUp);
}

bool GxContactIsElemUpContact(GxContact* contact, sElement* self) {
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
