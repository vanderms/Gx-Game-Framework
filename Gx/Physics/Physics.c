#include "../Util/Util.h"
#include "../Physics/Physics.h"
#include "../Qtree/Qtree.h"
#include "../Scene/Scene.h"
#include "../Element/Element.h"
#include "../Map/Map.h"
#include "../Array/Array.h"
#include "../List/List.h"
#include <string.h>
#include "../App/App.h"




typedef struct sPhysics {
	sScene* scene;	
	sArray* contacts;
	sQtree* fixed;
	sQtree* dynamic;	

	//buffers
	sRect* walls;
	sArray* emdstack;
	sArray* mvstack;
	sArray* cntend;
	int depth;
} sPhysics;

typedef struct sContact {
	Uint32 hash;
	sElement* colliding;
	sElement* collided;
	Uint32 direction;	
	int amove;
	bool effective;
	bool prevented;
} sContact;


//... Prototypes
static inline void destroyContact(sContact* self);
static void physicsCheckGround(sElement * other);

//... Constructor and destructor
static sPhysics* create(sScene* scene) {
	sPhysics* self = malloc(sizeof(sPhysics));
	nUtil->assertAlloc(self);
	self->scene = scene;	
	self->contacts = nArray->create();
	sSize size = nScene->size(scene);
	int len = size.w > size.h ? size.w + 2 : size.h + 2;		
	self->dynamic =nQtree->create(NULL, (sRect) { -1, -1, len, len });
	self->fixed =nQtree->create(NULL, (sRect) { -1, -1, len, len });	

	//buffers
	self->walls = NULL;
	self->emdstack = nArray->create();
	self->mvstack = nArray->create();
	self->cntend = nArray->create();
	self->depth = 0;
	return self;
}

static void destroy(sPhysics* self) {		
	if (self) {		
		for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
			sContact* contact = nArray->at(self->contacts, i);			
			destroyContact(contact);
		}
		nArray->destroy(self->contacts);
		nArray->destroy(self->emdstack);
		nArray->destroy(self->mvstack);
		nArray->destroy(self->cntend);
		nQtree->destroy(self->dynamic);
		nQtree->destroy(self->fixed);
		free(self);
	}
}

typedef struct EmData {
	sElement* self;
	sRect trajetory;
	sRect requestedPos;
	sRect previousPos;
	sArray* contacts;
}EmData;

static EmData* createEmData(sElement* elem) {
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

static void destroyEmData(EmData* self) {
	if (self) {
		nArray->destroy(self->contacts);
		free(self);
	}
}

static sContact* createContact(sElement* self, sElement* other, int amove, Uint32 direction) {
	sContact* contact = malloc(sizeof(sContact));	
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

static void destroyContact(sContact* self) {
	if (self) {
		sScene* scene = nElem->scene(self->colliding);
		if (self->effective && !nScene->hasStatus(scene, nUtil->status->UNLOADING)) {			
			nElem->body->p->removeContact(self->colliding, self);
			nElem->body->p->removeContact(self->collided, self);
			nArray->removeByValue(nScene->p->getPhysics(scene)->contacts, self);
		}
		free(self);
	}
}

//... PHYSIC STATIC METHODS PROTOTYPES
static sVector physicsProcessMovementData(sPhysics * self);
static void physicsApplyGravity(sPhysics * self, sElement * elem);
static void physicsCheckContactEnd(sPhysics* self, sElement* element);
static void physicsMoveElement_(sElement* element);
static bool physicsAddContact(sPhysics* self, sContact* contact);
static void physicsCheckCollision(sElement* other);
static bool contactIsEqual(sContact* lhs, sContact* rhs);

//... METHODS
static void update(sPhysics* self) {
	
	sRect area = *nElem->position(nScene->getCamera(self->scene));
	area.x -= 64;
	area.y -= 64;
	area.w += 128;
	area.h += 128;
	
	sArray* temp = nArray->create();
	nQtree->getAllElementsInArea(self->dynamic, &area, temp, true);
	for (Uint32 i = 0; i < nArray->size(temp); i++) {
		physicsMoveElement_(nQtree->getElem(nArray->at(temp, i)));
	}
	nArray->destroy(temp);	
	nArray->clean(self->mvstack);
}

static void insert(sPhysics* self, sElement* element) {
	if (!nElem->hasBody(element)) { return; }
	sQtreeElem* fixed = nElem->body->p->getQtreeElemFixed(element);
	nQtree->insert(self->fixed, fixed);
	if (nElem->body->isDynamic(element)) {
		sQtreeElem* dynamic = nElem->body->p->getQtreeElemDynamic(element);
		nQtree->insert(self->dynamic, dynamic);
	}
}

static void removeElem(sPhysics* self, sElement* element) {
	if (!nElem->hasBody(element)) return;
	sQtreeElem* fixed = nElem->body->p->getQtreeElemFixed(element);
	nQtree->remove(self->fixed, fixed);
	
	if (nElem->body->isDynamic(element)) {
		sQtreeElem* dynamic = nElem->body->p->getQtreeElemDynamic(element);
		nQtree->remove(self->dynamic, dynamic);
	}

	sArray* contacts = nArray->create();
	
	for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
		sContact* contact = nArray->at(self->contacts, i);
		if (contact->colliding == element || contact->collided == element) {			
			nArray->push(contacts, contact, destroyContact);
		}
	}
	//the contacts are destroyed with the array
	nArray->destroy(contacts);
}

static void updateElem(sPhysics* self, sElement* element, sRect previousPos) {	
	if(nElem->hasBody(element)){
		if (nElem->body->isDynamic(element)) {
			sQtreeElem* dynamic = nElem->body->p->getQtreeElemDynamic(element);
			nQtree->update(self->dynamic, dynamic, previousPos);
		}	
		sQtreeElem* fixed = nElem->body->p->getQtreeElemFixed(element);
		nQtree->update(self->fixed, fixed, previousPos);
	}	
}

static sVector moveByElem(sPhysics* self, sElement* element) {
	physicsMoveElement_(element);
	return *(sVector*) nArray->last(self->mvstack); 
}

static void physicsMoveElement_(sElement* element) {

	nUtil->assertState(!nElem->body->p->mcFlag(element));
	sPhysics* physics = nScene->p->getPhysics(nElem->scene(element));			
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
	sArray* temp = nArray->create();
	nQtree->getAllElementsInArea(physics->fixed, &emdata->trajetory, temp, true);
	for (Uint32 i = 0; i < nArray->size(temp); i++) {		
		physicsCheckCollision(nQtree->getElem(nArray->at(temp, i)));
	}	
	sVector* vec = nUtil->assertAlloc(malloc(sizeof(sVector)));		
	*vec = physicsProcessMovementData(physics);
	nArray->push(physics->mvstack, vec, free);

	if (nScene->hasGravity(physics->scene) &&
		nElem->body->maxgvel(element)
	){				
		for (Uint32 i = 0; i < nArray->size(temp); i++) {
			physicsCheckGround(nQtree->getElem(nArray->at(temp, i)));
		}
	}
	nArray->destroy(temp);
	
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

static void physicsApplyGravity(sPhysics* self, sElement* elem) {	

	int maxgvel = nElem->body->maxgvel(elem);
	int gravity = nScene->gravity(self->scene);
	int vely = nElem->body->vely(elem);
	if (gravity && (maxgvel < vely) && !nElem->body->isOnGround(elem)) {
		double acceleration = gravity / 60.0;
		nElem->body->accelerate(elem, 0, acceleration);
	}	
}

static void physicsApplyFriction(sPhysics* self, sElement* element, sVector move) {	
	if (nScene->hasGravity(self->scene) && nElem->body->hasFriction(element)) {
		move = (sVector) { move.x, move.y < 0 ? move.y : 0 };
		sArray* up = nElem->body->getContacts(element, nContact->UP);
		for (Uint32 i = 0; i < nArray->size(up); i++){
		sContact* contact = nArray->at(up, i); 
			sElement* other = contact->colliding == element ? contact->collided : contact->colliding;
			nElem->body->move(other, move, false);
		}
	}
}

static void physicsCheckCollision(sElement* other) {	
	
	//create alias	
	sPhysics* physics = nScene->p->getPhysics(nElem->scene(other));
	EmData* emdata = nArray->last(physics->emdstack);
	sElement* self = emdata->self;	

	if (self == other) return;
	if (!(nElem->body->cmask(self) & nElem->body->cmask(other))) return;

	//create alias
	sVector v = nElem->body->velocity(self);
	const sRect* s = nElem->position(self);
	const sRect* o = nElem->position(other);		

	if (!emdata->contacts) emdata->contacts = nArray->create();

	//check horizontal axis
	if (v.x > 0) {
		int amove = o->x - (s->x + s->w);
		if (amove >= 0 && amove < v.x) {
			nArray->push(emdata->contacts, createContact(self, other, amove, nContact->RIGHT), NULL);
		}
	}
	else if (v.x < 0) {
		int amove = (o->x + o->w) - s->x;
		if (amove <= 0 && amove > v.x) {
			nArray->push(emdata->contacts, createContact(self, other, amove, nContact->LEFT), NULL);
		}		
	}

	//check vertical axis
	if (v.y > 0) {
		int amove = o->y - (s->y + s->h);
		if (amove >= 0 && amove < v.y) {
			nArray->push(emdata->contacts, createContact(self, other, amove, nContact->UP), NULL);
		}
	}
	else if (v.y < 0) {
		int amove = (o->y + o->h) - s->y;
		if (amove <= 0 && amove > v.y) {
			nArray->push(emdata->contacts, createContact(self, other, amove, nContact->DOWN), NULL);
		}		
	}	
}

static sVector physicsProcessMovementData(sPhysics* self) {	

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
		sContact* contact = nArray->at(emdata->contacts, i);
		nScene->p->onPreContact(self->scene, contact);
		if (contact->prevented) { continue; }

		int spref = nElem->body->preference(contact->colliding);
		int opref = nElem->body->preference(contact->collided);
		sRect spos = *nElem->position(contact->colliding);
		sRect opos = *nElem->position(contact->collided);
		
		if (contact->direction == nContact->RIGHT && move.x >= contact->amove) {			
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
		else if (contact->direction == nContact->LEFT && move.x <= contact->amove) {
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
		else if (contact->direction == nContact->UP && move.y >= contact->amove) {
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
		else if (contact->direction == nContact->DOWN && move.y <= contact->amove) {
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
		sContact* contact = nArray->at(emdata->contacts, i); 		
		int spref = nElem->body->preference(contact->colliding);
		int opref = nElem->body->preference(contact->collided);
		sRect spos = *nElem->position(contact->colliding);
		sRect opos = *nElem->position(contact->collided);

		if (opref >= spref) changeVelocity = true;

		if ((contact->prevented) && 
			SDL_HasIntersection(&spos, &opos)) {			
			physicsAddContact(self, contact);			
		}
		else if ((contact->direction == nContact->RIGHT || contact->direction == nContact->LEFT) && (move.x == contact->amove)) {
			if (xres < nElem->body->restitution(contact->collided))
				xres = nElem->body->restitution(contact->collided);
			physicsAddContact(self, contact);	
		}
		else if ((contact->direction == nContact->UP || contact->direction == nContact->DOWN) && (move.y == contact->amove)) {
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
		sContact* contact = nArray->at(self->contacts, previousSize);
		nScene->p->onContactBegin(self->scene, contact);
		previousSize++;
	}
	return move;
}

static bool physicsAddContact(sPhysics* self, sContact* contact) {	
	
	bool contains = false;

	for (Uint32 i = 0; i < nArray->size(self->contacts); i++){
		sContact* buffer = nArray->at(self->contacts, i);		
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

static void physicsCheckContactEnd(sPhysics* self, sElement* element) {
		
	sArray* contactsToRemove = nArray->create();
	sList* allContacts = nElem->body->p->getContactList(element);
	
	for (sContact* contact = nList->begin(allContacts); contact != NULL;
		contact = nList->next(allContacts))
	{	
		sRect spos = *nElem->position(contact->colliding);
		sRect opos = *nElem->position(contact->collided);
				
		if (contact->prevented){			
			if (!SDL_HasIntersection(&spos, &opos)) {
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
		else if((bool) (contact->direction == nContact->RIGHT || contact->direction == nContact->LEFT)){
			bool notInTheSameRow = (spos.y >= opos.y + opos.h || spos.y + spos.h <= opos.y);
			bool touchingOnXAxis = (contact->direction ==  nContact->RIGHT) ?
				spos.x + spos.w == opos.x : spos.x == opos.x + opos.w;
			if (notInTheSameRow || !touchingOnXAxis) {
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
			}			
		}
		else if((bool) (contact->direction == nContact->UP || contact->direction == nContact->DOWN)){
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
			bool touchingOnYAxis = (contact->direction == nContact->UP) ?
				spos.y + spos.h == opos.y : spos.y == opos.y + opos.h;
			if (notInTheSameColumn || !touchingOnYAxis) {
				nArray->push(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
	}
	
	for (Uint32 i = 0; i < nArray->size(contactsToRemove); i++){
		sContact* contact = nArray->at(contactsToRemove, i);		
		nScene->p->onContactEnd(self->scene, contact);		
	}
	
	//... sArray destructor delete contacts //-> and sContact sDtor remove it from arrays.
	nArray->destroy(contactsToRemove);
}

static void physicsCheckGround(sElement* other) {
	sPhysics* physics = nScene->p->getPhysics(nElem->scene(other));
	EmData* emdata = nArray->last(physics->emdstack);
	sElement* self = emdata->self;
	
	const sRect* s = nElem->position(self);
	const sRect* o = nElem->position(other);

	bool samecolumn =  !(s->x >= o->x + o->w || s->x + s->w <= o->x);
	bool ytouching =  (s->y == o->y + o->h);
	if (samecolumn && ytouching) {
		sContact* contact = createContact(self, other, 0, nContact->DOWN);
		nScene->p->onPreContact(physics->scene, contact);
		if (physicsAddContact(physics, contact)) {
			nScene->p->onContactBegin(physics->scene, contact);
		}
	}
}

static void createWalls(sPhysics* self) {
	
	sSize size = nScene->size(self->scene);
	sIni ini = {
		.className = "__WALL__",				
		.display = nElem->display->NONE,
		.body = nElem->body->FIXED,		
	};
	
	ini.position = &(sRect) { -1, -1, size.w + 2, 1 };
	nElem->body->setCmask(nElem->create(&ini), nElem->body->CMASK_ALL);

	ini.position = &(sRect) {size.w, -1, 1, size.h + 2 };
	nElem->body->setCmask(nElem->create(&ini), nElem->body->CMASK_ALL);

	ini.position = &(sRect) { -1, size.h, size.w + 2, 1 };
	nElem->body->setCmask(nElem->create(&ini), nElem->body->CMASK_ALL);

	ini.position = &(sRect) { -1, -1, 1, size.h + 2  };
	nElem->body->setCmask(nElem->create(&ini), nElem->body->CMASK_ALL);
}


static sElement* colliding(sContact* contact) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return contact->colliding;
}

static sElement* collided(sContact* contact) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return contact->collided;
}

static bool isBetween(sContact* contact, sElement* self, sElement* other) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	bool s = self ? contact->colliding == self : true;
	bool o = other ? contact->collided == other : true;
	return s && o;
}

static bool hasElem(sContact* contact, sElement* element) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return (contact->colliding == element || contact->collided == element);
}


static sElement* getOpposite(sContact* contact, sElement* self) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	nUtil->assertArgument(contact->colliding == self || contact->collided == self);
	return (contact->colliding == self ? contact->collided : contact->colliding);
}

static bool hasDirection(sContact* contact, Uint32 direction) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return contact->direction & direction;
}

static Uint32 direction(sContact* contact) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return contact->direction;
}

static bool wasAllowed(sContact* contact) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return contact->prevented;
}

static void allowCollision(sContact* contact) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	contact->prevented = true;
}

static bool isFromRight(sContact* contact, sElement* self) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return (contact->colliding == self && contact->direction == nContact->RIGHT) ||
		(contact->collided == self && contact->direction == nContact->LEFT);
}

static bool isFromLeft(sContact* contact, sElement* self) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return (contact->colliding == self && contact->direction == nContact->LEFT) ||
		(contact->collided == self && contact->direction == nContact->RIGHT);
}

static bool isFromBelow(sContact* contact, sElement* self) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return (contact->colliding == self && contact->direction == nContact->DOWN) ||
		(contact->collided == self && contact->direction == nContact->UP);
}

static bool isFromAbove(sContact* contact, sElement* self) {
	nUtil->assertNullPointer(contact);
	nUtil->assertHash(contact->hash == nUtil->hash->CONTACT);
	return (contact->colliding == self && contact->direction == nContact->UP) ||
		(contact->collided == self && contact->direction == nContact->DOWN);
}

static void oneWayPlatformCbk(sEvent* e) {

	if(!nContact->hasDirection(e->contact, nContact->DOWN)){
		nContact->allowCollision(e->contact);
	}
}

static inline bool contactIsEqual(sContact* lhs, sContact* rhs) {
	return (
		lhs->colliding == rhs->colliding && 
		lhs->collided == rhs->collided && 
		lhs->direction == rhs->direction
	);
}

const struct sPhysicsNamespace* const nPhysics =  &(struct sPhysicsNamespace){
	//...
	.create = create,
	.destroy = destroy,

	//...
	.update = update,
	.insert = insert,
	.remove = removeElem,
	.updateElem = updateElem,
	.moveByElem = moveByElem,
	.createWalls = createWalls,
};

const struct sContactNamespace* const nContact = &(struct sContactNamespace) {		
	.colliding  = colliding,
	.collided = collided,
	.isBetween = isBetween,
	.hasElem = hasElem,
	.hasDirection = hasDirection,	
	.direction = direction,
	.getOpposite = getOpposite,
	.wasAllowed = wasAllowed,
	.allowCollision = allowCollision,
	.isFromRight = isFromRight,
	.isFromLeft = isFromLeft,
	.isFromBelow = isFromBelow,
	.isFromAbove = isFromAbove,
	.oneWayPlatformCbk = oneWayPlatformCbk,
	.RIGHT = 1 << 0,
	.LEFT = 1 << 1,
	.HORIZONTAL = 1 << 0 | 1 << 1,
	.UP = 1 << 2,
	.DOWN = 1 << 3,
	.VERTICAL = 1 << 2 | 1 << 3,
	.ALLOWED = 1 << 4,
	.ALL = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4,
};
