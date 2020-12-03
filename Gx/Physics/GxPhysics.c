#include "../Utilities/Util.h"
#include "../Physics/GxPhysics.h"
#include "../Quadtree/GxQuadtree.h"
#include "../Scene/Scene.h"
#include "../Element/Element.h"
#include "../Map/GxMap.h"
#include "../Array/Array.h"
#include "../List/List.h"
#include <string.h>
#include "../App/App.h"




typedef struct GxPhysics {
	sScene* scene;	
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
GxPhysics* GxCreatePhysics_(sScene* scene) {
	GxPhysics* self = malloc(sizeof(GxPhysics));
	nUtil->assertAlloc(self);
	self->scene = scene;	
	self->contacts = nArray->create();
	sSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w + 2 : size.h + 2;		
	self->dynamic = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "dynamic");
	self->fixed = GxCreateQtree_(NULL, (SDL_Rect) { -1, -1, length, length }, "fixed");	

	//buffers
	self->walls = NULL;
	self->emdstack = nArray->create();
	self->mvstack = nArray->create();
	self->cntend = nArray->create();
	self->depth = 0;
	return self;
}

void GxDestroyPhysics_(GxPhysics* self) {		
	if (self) {		
		for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
			GxContact* contact = nArray->at(self->contacts, i);			
			destroyContact(contact);
		}
		nArray->destroy(self->contacts);
		nArray->destroy(self->emdstack);
		nArray->destroy(self->mvstack);
		nArray->destroy(self->cntend);
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
	self->requestedPos = self->previousPos = *nElem->position(elem);	
	self->requestedPos.x = self->previousPos.x + nElem->body->velx(elem);
	self->requestedPos.y = self->previousPos.y + nElem->body->vely(elem);
	SDL_UnionRect(&self->previousPos, &self->requestedPos, &self->trajetory);
	self->contacts = NULL;
	return self;
}

static inline void destroyEmData(EmData* self) {
	if (self) {
		nArray->destroy(self->contacts);
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
		sScene* scene = nElem->scene(self->colliding);
		if (self->effective && !GxSceneHasStatus(scene, GxStatusUnloading)) {			
			nElem->body->p->removeContact(self->colliding, self);
			nElem->body->p->removeContact(self->collided, self);
			nArray->removeByValue(GxSceneGetPhysics(scene)->contacts, self);
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
	
	SDL_Rect area = *nElem->position(GxSceneGetCamera(self->scene));
	area.x -= 64;
	area.y -= 64;
	area.w += 128;
	area.h += 128;
	
	GxQtreeIterate_(self->dynamic, area, physicsMoveElement_, true);
	nArray->clean(self->mvstack);
}

void GxPhysicsInsertElement_(GxPhysics* self, sElement* element) {
	if (!nElem->hasBody(element)) { return; }
	GxQtreeInsert_(self->fixed, element);
	if (nElem->body->isDynamic(element)) {
		GxQtreeInsert_(self->dynamic, element);
	}
}

void GxPhysicsRemoveElement_(GxPhysics* self, sElement* element) {
	if (!nElem->hasBody(element)) return;
	GxQtreeRemove_(self->fixed, element);
	
	if (nElem->body->isDynamic(element)) {
		GxQtreeRemove_(self->dynamic, element);
	}

	sArray* contacts = nArray->create();
	
	for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
		GxContact* contact = nArray->at(self->contacts, i);
		if (contact->colliding == element || contact->collided == element) {			
			nArray->push(contacts, contact, destroyContact);
		}
	}
	//the contacts are destroyed with the array
	nArray->destroy(contacts);
}

void GxPhysicsUpdateElementPosition_(GxPhysics* self, sElement* element, SDL_Rect previousPos) {	
	if(nElem->hasBody(element)){
		if (nElem->body->isDynamic(element)) {
			GxQtreeUpdate_(self->dynamic, element, previousPos);
		}	
		GxQtreeUpdate_(self->fixed, element, previousPos);
	}	
}

sVector GxPhysicsMoveCalledByElem_(GxPhysics* self, sElement* element) {
	physicsMoveElement_(element);
	return *(sVector*) nArray->last(self->mvstack); 
}

static inline void physicsMoveElement_(sElement* element) {

	nUtil->assertState(!nElem->body->p->mcFlag(element));
	GxPhysics* physics = GxSceneGetPhysics(nElem->scene(element));			
	physicsApplyGravity(physics, element);
	bool cantmove = nElem->body->p->movFlag(element) || !nElem->body->isMoving(element);		
	if (cantmove) {
		sVector* vector = malloc(sizeof(sVector));
		nUtil->assertAlloc(vector);
		*vector = (sVector){ 0, 0 };
		nArray->push(physics->mvstack, vector, free);		
		return;
	}	
	nElem->body->p->setMovFlag(element, true);	
	physics->depth++;
	nArray->push(physics->cntend, element, NULL);
				
	EmData* emdata = createEmData(element);
	nArray->push(physics->emdstack, emdata, (sDtor) destroyEmData);
	GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckCollision, true);
	sVector* vec = malloc(sizeof(sVector));
	nUtil->assertAlloc(vec);
	
	*vec = physicsProcessMovementData(physics);
	nArray->push(physics->mvstack, vec, free);

	if (GxSceneHasGravity(physics->scene) && nElem->body->maxgvel(element)) {		
		GxQtreeIterate_(physics->fixed, emdata->trajetory, physicsCheckGround, true);
	}
	
	nArray->remove(physics->emdstack, nArray->size(physics->emdstack) - 1);
	nElem->body->p->setMovFlag(element, false);	
	physics->depth--;
	if (physics->depth == 0) {
		for (Uint32 i = 0; i < nArray->size(physics->cntend); i++){
			sElement* elem = nArray->at(physics->cntend, i);
			physicsCheckContactEnd(physics, elem);			
		}
		nArray->clean(physics->cntend);
	}
}

static inline void physicsApplyGravity(GxPhysics* self, sElement* elem) {	

	int maxgvel = nElem->body->maxgvel(elem);
	int gravity = GxSceneGetGravity(self->scene);
	int vely = nElem->body->vely(elem);
	if (gravity && (maxgvel < vely) && !nElem->body->isOnGround(elem)) {
		double acceleration = gravity / 60.0;
		nElem->body->accelerate(elem, 0, acceleration);
	}	
}

static inline void physicsApplyFriction(GxPhysics* self, sElement* element, sVector move) {	
	if (GxSceneHasGravity(self->scene) && nElem->body->hasFriction(element)) {
		move = (sVector) { move.x, move.y < 0 ? move.y : 0 };
		sArray* up = nElem->body->getContacts(element, GxContactUp);
		for (Uint32 i = 0; i < nArray->size(up); i++){
		GxContact* contact = nArray->at(up, i); 
			sElement* other = contact->colliding == element ? contact->collided : contact->colliding;
			nElem->body->move(other, move, false);
		}
	}
}

static inline void physicsCheckCollision(sElement* other) {	
	
	//create alias	
	GxPhysics* physics = GxSceneGetPhysics(nElem->scene(other));
	EmData* emdata = nArray->last(physics->emdstack);
	sElement* self = emdata->self;	

	if (self == other) return;
	if (!(nElem->body->cmask(self) & nElem->body->cmask(other))) return;

	//create alias
	sVector v = nElem->body->velocity(self);
	const SDL_Rect* s = nElem->position(self);
	const SDL_Rect* o = nElem->position(other);	
	
	if (SDL_HasIntersection(&emdata->trajetory, o)) {

		if (!emdata->contacts) emdata->contacts = nArray->create();

		//check horizontal direction
		if (v.x > 0) {
			int amove = o->x - (s->x + s->w);
			if (amove >= 0 && amove < v.x) {
				nArray->push(emdata->contacts, createContact(self, other, amove, GxContactRight), NULL);
			}
		}
		else if (v.x < 0) {
			int amove = (o->x + o->w) - s->x;
			if (amove <= 0 && amove > v.x) {
				nArray->push(emdata->contacts, createContact(self, other, amove, GxContactLeft), NULL);
			}		
		}

		//check vertical direction
		if (v.y > 0) {
			int amove = o->y - (s->y + s->h);
			if (amove >= 0 && amove < v.y) {
				nArray->push(emdata->contacts, createContact(self, other, amove, GxContactUp), NULL);
			}
		}
		else if (v.y < 0) {
			int amove = (o->y + o->h) - s->y;
			if (amove <= 0 && amove > v.y) {
				nArray->push(emdata->contacts, createContact(self, other, amove, GxContactDown), NULL);
			}		
		}		
	}
}

static inline sVector physicsProcessMovementData(GxPhysics* self) {	

	//get emdata
	EmData* emdata = nArray->last(self->emdstack);

	//gets self velocity
	sVector move = nElem->body->velocity(emdata->self);

	//if there is no contact, just move the element
	if (!emdata->contacts) {			
		nElem->p->updatePosition(emdata->self, move);
		physicsApplyFriction(self, emdata->self, move);
		return move;
	}
	
	bool horizontalCollision = false, verticalCollision = false;

	for (Uint32 i = 0; i < nArray->size(emdata->contacts); i++){
		GxContact* contact = nArray->at(emdata->contacts, i);
		GxSceneOnPreContact_(self->scene, contact);
		if (contact->prevented) { continue; }

		int spref = nElem->body->preference(contact->colliding);
		int opref = nElem->body->preference(contact->collided);
		SDL_Rect spos = *nElem->position(contact->colliding);
		SDL_Rect opos = *nElem->position(contact->collided);
		
		if (contact->direction == GxContactRight && move.x >= contact->amove) {			
			if (spref > opref) {
				nElem->body->setPreference(contact->collided, spref);
				nElem->body->move(contact->collided, (sVector) { (spos.x + move.x + spos.w) - opos.x, 0 }, false);				
				nElem->body->setPreference(contact->collided, opref);
				contact->amove = (opos.x - (spos.x + spos.w));
			}			
			if(move.x >= contact->amove) {
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == GxContactLeft && move.x <= contact->amove) {
			if (spref > opref) {
				nElem->body->setPreference(contact->collided, spref);
				nElem->body->move(contact->collided, (sVector) {(spos.x + move.x) - (opos.x + opos.w), 0 }, false);				
				nElem->body->setPreference(contact->collided, opref);
				contact->amove = ((opos.x + opos.w) - spos.x);
			}			
			if(move.x <= contact->amove){
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == GxContactUp && move.y >= contact->amove) {
			if (spref > opref) {				
				nElem->body->setPreference(contact->collided, spref);
				nElem->body->move(contact->collided, (sVector) { 0, (spos.y + move.y + spos.h) - opos.y}, false);				
				nElem->body->setPreference(contact->collided, opref);
				contact->amove = opos.y - (spos.y + spos.h);			
			}			
			if(move.y >= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
		else if (contact->direction == GxContactDown && move.y <= contact->amove) {
			if (spref > opref) {	
				nElem->body->setPreference(contact->collided, spref);
				nElem->body->move(contact->collided, (sVector) {0, (spos.y + move.y) - (opos.y + opos.h)}, false);				
				nElem->body->setPreference(contact->collided, opref);
				contact->amove = (opos.y + opos.h) - spos.y;
			}			
			if(move.y <= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
	}

	//move entity to new position	
	nElem->p->updatePosition(emdata->self, move);
	physicsApplyFriction(self, emdata->self, move);

	//now check effective collisions and add to self collision array
	Uint32 previousSize = nArray->size(self->contacts);

	double xres = 0.0, yres = 0.0; //restitution in direction x and y
	bool changeVelocity = false;

	for (Uint32 i = 0; i < nArray->size(emdata->contacts); i++){
		GxContact* contact = nArray->at(emdata->contacts, i); 		
		int spref = nElem->body->preference(contact->colliding);
		int opref = nElem->body->preference(contact->collided);
		SDL_Rect spos = *nElem->position(contact->colliding);
		SDL_Rect opos = *nElem->position(contact->collided);

		if (opref >= spref) changeVelocity = true;

		if ((contact->prevented) && 
			SDL_HasIntersection(&spos, &opos)) {			
			physicsAddContact(self, contact);			
		}
		else if ((contact->direction == GxContactRight || contact->direction == GxContactLeft) && (move.x == contact->amove)) {
			if (xres < nElem->body->restitution(contact->collided))
				xres = nElem->body->restitution(contact->collided);
			physicsAddContact(self, contact);	
		}
		else if ((contact->direction == GxContactUp || contact->direction == GxContactDown) && (move.y == contact->amove)) {
			if (yres < nElem->body->restitution(contact->collided))
				yres = nElem->body->restitution(contact->collided);
			physicsAddContact(self, contact);
		}
		else {
			destroyContact(contact);
		}
	}
		
	if (changeVelocity) {		
		if (horizontalCollision) {
			nElem->body->p->applyHozElasticity(emdata->self, xres);			
		}
		if (verticalCollision) {
			nElem->body->p->applyVetElasticity(emdata->self, yres);
		}		
	}

		//notify collision callback handler
	while (previousSize < nArray->size(self->contacts)) {
		GxContact* contact = nArray->at(self->contacts, previousSize);
		GxSceneOnContactBegin_(self->scene, contact);
		previousSize++;
	}
	return move;
}

bool physicsAddContact(GxPhysics* self, GxContact* contact) {	
	
	bool contains = false;

	for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
		GxContact* buffer = nArray->at(self->contacts, i);		
		if (contactIsEqual(contact, buffer)) {
			contains = true;
			break;
		}
	}

	if (!contains) {		
		nElem->body->p->addContact(contact->colliding, contact);
		nElem->body->p->addContact(contact->collided, contact);		
		contact->effective = true;
		nArray->push(self->contacts, contact, NULL);			
		return true;
	}

	destroyContact(contact);
	return false;	
}

void physicsCheckContactEnd(GxPhysics* self, sElement* element) {
		
	sArray* contactsToRemove = nArray->create();
	sList* allContacts = nElem->body->p->getContactList(element);
	
	for (GxContact* contact = nList->begin(allContacts); contact != NULL;
		contact = nList->next(allContacts))
	{	
		SDL_Rect spos = *nElem->position(contact->colliding);
		SDL_Rect opos = *nElem->position(contact->collided);
				
		if (contact->prevented){			
			if (!SDL_HasIntersection(&spos, &opos)) {
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
		else if((bool) (contact->direction == GxContactRight || contact->direction == GxContactLeft)){
			bool notInTheSameRow = (spos.y >= opos.y + opos.h || spos.y + spos.h <= opos.y);
			bool touchingOnXAxis = (contact->direction ==  GxContactRight) ?
				spos.x + spos.w == opos.x : spos.x == opos.x + opos.w;
			if (notInTheSameRow || !touchingOnXAxis) {
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
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
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
	}
	
	for (Uint32 i = 0; i < nArray->size(contactsToRemove); i++){
		GxContact* contact = nArray->at(contactsToRemove, i);		
		GxSceneOnContactEnd_(self->scene, contact);		
	}
	
	//... sArray destructor delete contacts //-> and GxContact sDtor remove it from arrays.
	nArray->destroy(contactsToRemove);
}

static void physicsCheckGround(sElement* other) {
	GxPhysics* physics = GxSceneGetPhysics(nElem->scene(other));
	EmData* emdata = nArray->last(physics->emdstack);
	sElement* self = emdata->self;
	
	const SDL_Rect* s = nElem->position(self);
	const SDL_Rect* o = nElem->position(other);

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
	
	sSize size = GxSceneGetSize(self->scene);
	sIni ini = {
		.className = "__WALL__",				
		.display = nElem->display->NONE,
		.body = nElem->body->FIXED,		
	};
	
	ini.position = &(SDL_Rect) { -1, -1, size.w + 2, 1 };
	nElem->body->setCmask(nElem->create(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) {size.w, -1, 1, size.h + 2 };
	nElem->body->setCmask(nElem->create(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) { -1, size.h, size.w + 2, 1 };
	nElem->body->setCmask(nElem->create(&ini), GxCmaskAll);

	ini.position = &(SDL_Rect) { -1, -1, 1, size.h + 2  };
	nElem->body->setCmask(nElem->create(&ini), GxCmaskAll);
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
