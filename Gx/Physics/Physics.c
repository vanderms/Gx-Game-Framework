#include "../Util/Util.h"
#include "../Physics/Physics.h"
#include "../Containers/Qtree/Qtree.h"
#include "../Scene/Scene.h"
#include "../Element/Element.h"
#include "../Containers/Map/Map.h"
#include "../Containers/Array/Array.h"
#include "../Containers/List/List.h"
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


const Uint32 nContact_RIGHT = 1 << 0;
const Uint32 nContact_LEFT = 1 << 1;
const Uint32 nContact_HORIZONTAL = 1 << 0 | 1 << 1;
const Uint32 nContact_UP = 1 << 2;
const Uint32 nContact_DOWN = 1 << 3;
const Uint32 nContact_VERTICAL = 1 << 2 | 1 << 3;
const Uint32 nContact_ALLOWED = 1 << 4;
const Uint32 nContact_ALL = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4;

//... Prototypes
static inline void destroyContact(sContact* self);
static void physicsCheckGround(sElement * other);
static sVector physicsProcessMovementData(sPhysics * self);
static void physicsApplyGravity(sPhysics * self, sElement * elem);
static void physicsCheckContactEnd(sPhysics* self, sElement* element);
static void physicsMoveElement_(sElement* element);
static bool physicsAddContact(sPhysics* self, sContact* contact);
static void physicsCheckCollision(sElement* other);
static bool contactIsEqual(sContact* lhs, sContact* rhs);

//... Constructor and destructor
sPhysics* nPhysicsCreate_(sScene* scene) {
	sPhysics* self = malloc(sizeof(sPhysics));
	nUtilAssertAlloc(self);
	self->scene = scene;	
	self->contacts = nArrayCreate();
	sSize size = nSceneSize(scene);
	int len = size.w > size.h ? size.w + 2 : size.h + 2;		
	self->dynamic =nQtreeCreate(NULL, (sRect) { -1, -1, len, len });
	self->fixed =nQtreeCreate(NULL, (sRect) { -1, -1, len, len });	

	//buffers
	self->walls = NULL;
	self->emdstack = nArrayCreate();
	self->mvstack = nArrayCreate();
	self->cntend = nArrayCreate();
	self->depth = 0;
	return self;
}

void nPhysicsDestroy_(sPhysics* self) {		
	if (self) {		
		for (Uint32 i = 0; i < nArraySize(self->contacts); i++){
			sContact* contact = nArrayAt(self->contacts, i);			
			destroyContact(contact);
		}
		nArrayDestroy(self->contacts);
		nArrayDestroy(self->emdstack);
		nArrayDestroy(self->mvstack);
		nArrayDestroy(self->cntend);
		nQtreeDestroy(self->dynamic);
		nQtreeDestroy(self->fixed);
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
	nUtilAssertAlloc(self);
	self->self = elem;
	self->requestedPos = self->previousPos = *nElemPosition(elem);	
	self->requestedPos.x = self->previousPos.x + nElemVelx(elem);
	self->requestedPos.y = self->previousPos.y + nElemVely(elem);
	SDL_UnionRect(&self->previousPos, &self->requestedPos, &self->trajetory);
	self->contacts = NULL;
	return self;
}

static void destroyEmData(EmData* self) {
	if (self) {
		nArrayDestroy(self->contacts);
		free(self);
	}
}

static sContact* createContact(sElement* self, sElement* other, int amove, Uint32 direction) {
	sContact* contact = malloc(sizeof(sContact));	
	nUtilAssertAlloc(contact);
	contact->hash = nUtil_HASH_CONTACT;
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
		sScene* scene = nElemScene(self->colliding);
		if (self->effective && !nSceneHasStatus(scene, nUtil_STATUS_UNLOADING)) {			
			nElemRemoveContact_(self->colliding, self);
			nElemRemoveContact_(self->collided, self);
			nArrayRemoveByValue(nSceneGetPhysics_(scene)->contacts, self);
		}
		free(self);
	}
}



//... 
void nPhysicsUpdate_(sPhysics* self) {
	
	sRect area = *nElemPosition(nSceneGetCamera(self->scene));
	area.x -= 64;
	area.y -= 64;
	area.w += 128;
	area.h += 128;
	
	sArray* temp = nArrayCreate();
	nQtreeGetElementsInArea(self->dynamic, &area, temp, true);
	for (Uint32 i = 0; i < nArraySize(temp); i++) {
		physicsMoveElement_(nQtreeGetElem(nArrayAt(temp, i)));
	}
	nArrayDestroy(temp);	
	nArrayClean(self->mvstack);
}

void nPhysicsInsert_(sPhysics* self, sElement* element) {
	if (!nElemHasBody(element)) { return; }
	sQtreeElem* fixed = nElemGetQtreeElemFixed_(element);
	nQtreeInsert(self->fixed, fixed);
	if (nElemIsBodyDynamic(element)) {
		sQtreeElem* dynamic = nElemGetQtreeElemDynamic_(element);
		nQtreeInsert(self->dynamic, dynamic);
	}
}

void nPhysicsRemoveElem_(sPhysics* self, sElement* element) {
	if (!nElemHasBody(element)) return;
	sQtreeElem* fixed = nElemGetQtreeElemFixed_(element);
	nQtreeRemove(self->fixed, fixed);
	
	if (nElemIsBodyDynamic(element)) {
		sQtreeElem* dynamic = nElemGetQtreeElemDynamic_(element);
		nQtreeRemove(self->dynamic, dynamic);
	}

	sArray* contacts = nArrayCreate();
	
	for (Uint32 i = 0; i < nArraySize(self->contacts); i++){
		sContact* contact = nArrayAt(self->contacts, i);
		if (contact->colliding == element || contact->collided == element) {			
			nArrayPush(contacts, contact, destroyContact);
		}
	}
	//the contacts are destroyed with the array
	nArrayDestroy(contacts);
}

void nPhysicsUpdateElem_(sPhysics* self, sElement* element, sRect previousPos) {	
	if(nElemHasBody(element)){
		if (nElemIsBodyDynamic(element)) {
			sQtreeElem* dynamic = nElemGetQtreeElemDynamic_(element);
			nQtreeUpdate(self->dynamic, dynamic, previousPos);
		}	
		sQtreeElem* fixed = nElemGetQtreeElemFixed_(element);
		nQtreeUpdate(self->fixed, fixed, previousPos);
	}	
}

sVector nPhysicsMoveByElem_(sPhysics* self, sElement* element) {
	physicsMoveElement_(element);
	return *(sVector*) nArrayLast(self->mvstack); 
}

static void physicsMoveElement_(sElement* element) {

	nUtilAssertState(!nElemMcFlag_(element));
	sPhysics* physics = nSceneGetPhysics_(nElemScene(element));			
	physicsApplyGravity(physics, element);
	bool cantmove = nElemMovFlag_(element) || !nElemIsMoving(element);		
	if (cantmove) {
		sVector* vector = malloc(sizeof(sVector));
		nUtilAssertAlloc(vector);
		*vector = (sVector){ 0, 0 };
		nArrayPush(physics->mvstack, vector, free);		
		return;
	}	
	nElemSetMovFlag_(element, true);	
	physics->depth++;
	nArrayPush(physics->cntend, element, NULL);
				
	EmData* emdata = createEmData(element);
	nArrayPush(physics->emdstack, emdata, (sDtor) destroyEmData);
	sArray* temp = nArrayCreate();
	nQtreeGetElementsInArea(physics->fixed, &emdata->trajetory, temp, true);
	for (Uint32 i = 0; i < nArraySize(temp); i++) {		
		physicsCheckCollision(nQtreeGetElem(nArrayAt(temp, i)));
	}	
	sVector* vec =nUtilAssertAlloc(malloc(sizeof(sVector)));		
	*vec = physicsProcessMovementData(physics);
	nArrayPush(physics->mvstack, vec, free);

	if (nSceneHasGravity(physics->scene) &&
		nElemMaxgvel(element)
	){				
		for (Uint32 i = 0; i < nArraySize(temp); i++) {
			physicsCheckGround(nQtreeGetElem(nArrayAt(temp, i)));
		}
	}
	nArrayDestroy(temp);
	
	nArrayRemove(physics->emdstack, nArraySize(physics->emdstack) - 1);
	nElemSetMovFlag_(element, false);	
	physics->depth--;
	if (physics->depth == 0) {
		for (Uint32 i = 0; i < nArraySize(physics->cntend); i++){
			sElement* elem = nArrayAt(physics->cntend, i);
			physicsCheckContactEnd(physics, elem);			
		}
		nArrayClean(physics->cntend);
	}
}

static void physicsApplyGravity(sPhysics* self, sElement* elem) {	

	int maxgvel = nElemMaxgvel(elem);
	int gravity = nSceneGravity(self->scene);
	int vely = nElemVely(elem);
	if (gravity && (maxgvel < vely) && !nElemIsOnGround(elem)) {
		double acceleration = gravity / 60.0;
		nElemAccelerate(elem, 0, acceleration);
	}	
}

static void physicsApplyFriction(sPhysics* self, sElement* element, sVector move) {	
	if (nSceneHasGravity(self->scene) && nElemHasFriction(element)) {
		move = (sVector) { move.x, move.y < 0 ? move.y : 0 };
		sArray* up = nElemGetContacts(element, nContact_UP);
		for (Uint32 i = 0; i < nArraySize(up); i++){
		sContact* contact = nArrayAt(up, i); 
			sElement* other = contact->colliding == element ? contact->collided : contact->colliding;
			nElemMove(other, move, false);
		}
	}
}

static void physicsCheckCollision(sElement* other) {	
	
	//create alias	
	sPhysics* physics = nSceneGetPhysics_(nElemScene(other));
	EmData* emdata = nArrayLast(physics->emdstack);
	sElement* self = emdata->self;	

	if (self == other) return;
	if (!(nElemCmask(self) & nElemCmask(other))) return;

	//create alias
	sVector v = nElemVelocity(self);
	const sRect* s = nElemPosition(self);
	const sRect* o = nElemPosition(other);		

	if (!emdata->contacts) emdata->contacts = nArrayCreate();

	//check horizontal axis
	if (v.x > 0) {
		int amove = o->x - (s->x + s->w);
		if (amove >= 0 && amove < v.x) {
			nArrayPush(emdata->contacts, createContact(self, other, amove, nContact_RIGHT), NULL);
		}
	}
	else if (v.x < 0) {
		int amove = (o->x + o->w) - s->x;
		if (amove <= 0 && amove > v.x) {
			nArrayPush(emdata->contacts, createContact(self, other, amove, nContact_LEFT), NULL);
		}		
	}

	//check vertical axis
	if (v.y > 0) {
		int amove = o->y - (s->y + s->h);
		if (amove >= 0 && amove < v.y) {
			nArrayPush(emdata->contacts, createContact(self, other, amove, nContact_UP), NULL);
		}
	}
	else if (v.y < 0) {
		int amove = (o->y + o->h) - s->y;
		if (amove <= 0 && amove > v.y) {
			nArrayPush(emdata->contacts, createContact(self, other, amove, nContact_DOWN), NULL);
		}		
	}	
}

static sVector physicsProcessMovementData(sPhysics* self) {	

	//get emdata
	EmData* emdata = nArrayLast(self->emdstack);

	//gets self velocity
	sVector move = nElemVelocity(emdata->self);

	//if there is no contact, just move the element
	if (!emdata->contacts) {			
		nElemUpdatePosition_(emdata->self, move);
		physicsApplyFriction(self, emdata->self, move);
		return move;
	}
	
	bool horizontalCollision = false, verticalCollision = false;

	for (Uint32 i = 0; i < nArraySize(emdata->contacts); i++){
		sContact* contact = nArrayAt(emdata->contacts, i);
		nSceneOnPreContact_(self->scene, contact);
		if (contact->prevented) { continue; }

		int spref = nElemPreference(contact->colliding);
		int opref = nElemPreference(contact->collided);
		sRect spos = *nElemPosition(contact->colliding);
		sRect opos = *nElemPosition(contact->collided);
		
		if (contact->direction == nContact_RIGHT && move.x >= contact->amove) {			
			if (spref > opref) {
				nElemSetPreference(contact->collided, spref);
				nElemMove(contact->collided, (sVector) { (spos.x + move.x + spos.w) - opos.x, 0 }, false);				
				nElemSetPreference(contact->collided, opref);
				contact->amove = (opos.x - (spos.x + spos.w));
			}			
			if(move.x >= contact->amove) {
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == nContact_LEFT && move.x <= contact->amove) {
			if (spref > opref) {
				nElemSetPreference(contact->collided, spref);
				nElemMove(contact->collided, (sVector) {(spos.x + move.x) - (opos.x + opos.w), 0 }, false);				
				nElemSetPreference(contact->collided, opref);
				contact->amove = ((opos.x + opos.w) - spos.x);
			}			
			if(move.x <= contact->amove){
				move.x = contact->amove;
				horizontalCollision = true;
			}
		}
		else if (contact->direction == nContact_UP && move.y >= contact->amove) {
			if (spref > opref) {				
				nElemSetPreference(contact->collided, spref);
				nElemMove(contact->collided, (sVector) { 0, (spos.y + move.y + spos.h) - opos.y}, false);				
				nElemSetPreference(contact->collided, opref);
				contact->amove = opos.y - (spos.y + spos.h);			
			}			
			if(move.y >= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
		else if (contact->direction == nContact_DOWN && move.y <= contact->amove) {
			if (spref > opref) {	
				nElemSetPreference(contact->collided, spref);
				nElemMove(contact->collided, (sVector) {0, (spos.y + move.y) - (opos.y + opos.h)}, false);				
				nElemSetPreference(contact->collided, opref);
				contact->amove = (opos.y + opos.h) - spos.y;
			}			
			if(move.y <= contact->amove){
				move.y = contact->amove;
				verticalCollision = true;
			}
		}
	}

	//move entity to new position	
	nElemUpdatePosition_(emdata->self, move);
	physicsApplyFriction(self, emdata->self, move);

	//now check effective collisions and add to self collision array
	Uint32 previousSize = nArraySize(self->contacts);

	double xres = 0.0, yres = 0.0; //restitution in direction x and y
	bool changeVelocity = false;

	for (Uint32 i = 0; i < nArraySize(emdata->contacts); i++){
		sContact* contact = nArrayAt(emdata->contacts, i); 		
		int spref = nElemPreference(contact->colliding);
		int opref = nElemPreference(contact->collided);
		sRect spos = *nElemPosition(contact->colliding);
		sRect opos = *nElemPosition(contact->collided);

		if (opref >= spref) changeVelocity = true;

		if ((contact->prevented) && 
			SDL_HasIntersection(&spos, &opos)) {			
			physicsAddContact(self, contact);			
		}
		else if ((contact->direction == nContact_RIGHT || contact->direction == nContact_LEFT) && (move.x == contact->amove)) {
			if (xres < nElemRestitution(contact->collided))
				xres = nElemRestitution(contact->collided);
			physicsAddContact(self, contact);	
		}
		else if ((contact->direction == nContact_UP || contact->direction == nContact_DOWN) && (move.y == contact->amove)) {
			if (yres < nElemRestitution(contact->collided))
				yres = nElemRestitution(contact->collided);
			physicsAddContact(self, contact);
		}
		else {
			destroyContact(contact);
		}
	}
		
	if (changeVelocity) {		
		if (horizontalCollision) {
			nElemApplyHozElasticity_(emdata->self, xres);			
		}
		if (verticalCollision) {
			nElemApplyVetElasticity_(emdata->self, yres);
		}		
	}

		//notify collision callback handler
	while (previousSize < nArraySize(self->contacts)) {
		sContact* contact = nArrayAt(self->contacts, previousSize);
		nSceneOnContactBegin_(self->scene, contact);
		previousSize++;
	}
	return move;
}

static bool physicsAddContact(sPhysics* self, sContact* contact) {	
	
	bool contains = false;

	for (Uint32 i = 0; i < nArraySize(self->contacts); i++){
		sContact* buffer = nArrayAt(self->contacts, i);		
		if (contactIsEqual(contact, buffer)) {
			contains = true;
			break;
		}
	}

	if (!contains) {		
		nElemAddContact_(contact->colliding, contact);
		nElemAddContact_(contact->collided, contact);		
		contact->effective = true;
		nArrayPush(self->contacts, contact, NULL);			
		return true;
	}

	destroyContact(contact);
	return false;	
}

static void physicsCheckContactEnd(sPhysics* self, sElement* element) {
		
	sArray* contactsToRemove = nArrayCreate();
	sList* allContacts = nElemGetContactList_(element);
	
	for (sContact* contact = nListBegin(allContacts); contact != NULL;
		contact = nListNext(allContacts))
	{	
		sRect spos = *nElemPosition(contact->colliding);
		sRect opos = *nElemPosition(contact->collided);
				
		if (contact->prevented){			
			if (!SDL_HasIntersection(&spos, &opos)) {
				nArrayPush(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
		else if((bool) (contact->direction == nContact_RIGHT || contact->direction == nContact_LEFT)){
			bool notInTheSameRow = (spos.y >= opos.y + opos.h || spos.y + spos.h <= opos.y);
			bool touchingOnXAxis = (contact->direction ==  nContact_RIGHT) ?
				spos.x + spos.w == opos.x : spos.x == opos.x + opos.w;
			if (notInTheSameRow || !touchingOnXAxis) {
				nArrayPush(contactsToRemove, contact, (sDtor) destroyContact);
			}			
		}
		else if((bool) (contact->direction == nContact_UP || contact->direction == nContact_DOWN)){
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
			bool touchingOnYAxis = (contact->direction == nContact_UP) ?
				spos.y + spos.h == opos.y : spos.y == opos.y + opos.h;
			if (notInTheSameColumn || !touchingOnYAxis) {
				nArrayPush(contactsToRemove, contact, (sDtor) destroyContact);
			}
		}
	}
	
	for (Uint32 i = 0; i < nArraySize(contactsToRemove); i++){
		sContact* contact = nArrayAt(contactsToRemove, i);		
		nSceneOnContactEnd_(self->scene, contact);		
	}
	
	//... sArray destructor delete contacts //-> and sContact sDtor remove it from arrays.
	nArrayDestroy(contactsToRemove);
}

static void physicsCheckGround(sElement* other) {
	sPhysics* physics = nSceneGetPhysics_(nElemScene(other));
	EmData* emdata = nArrayLast(physics->emdstack);
	sElement* self = emdata->self;
	
	const sRect* s = nElemPosition(self);
	const sRect* o = nElemPosition(other);

	bool samecolumn =  !(s->x >= o->x + o->w || s->x + s->w <= o->x);
	bool ytouching =  (s->y == o->y + o->h);
	if (samecolumn && ytouching) {
		sContact* contact = createContact(self, other, 0, nContact_DOWN);
		nSceneOnPreContact_(physics->scene, contact);
		if (physicsAddContact(physics, contact)) {
			nSceneOnContactBegin_(physics->scene, contact);
		}
	}
}

void nPhysicsCreateWalls_(sPhysics* self) {
	
	sSize size = nSceneSize(self->scene);
	sIni ini = {
		.className = "__WALL__",				
		.display = nElem_DISPLAY_NONE,
		.body = nElem_BODY_FIXED,		
	};
	
	ini.position = &(sRect) { -1, -1, size.w + 2, 1 };
	nElemSetCmask(nElemCreate(&ini), nElem_CMASK_ALL);

	ini.position = &(sRect) {size.w, -1, 1, size.h + 2 };
	nElemSetCmask(nElemCreate(&ini), nElem_CMASK_ALL);

	ini.position = &(sRect) { -1, size.h, size.w + 2, 1 };
	nElemSetCmask(nElemCreate(&ini), nElem_CMASK_ALL);

	ini.position = &(sRect) { -1, -1, 1, size.h + 2  };
	nElemSetCmask(nElemCreate(&ini), nElem_CMASK_ALL);
}


sElement* nContactColliding(sContact* contact) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return contact->colliding;
}

sElement* nContactCollided(sContact* contact) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return contact->collided;
}

bool nContactIsBetween(sContact* contact, sElement* self, sElement* other) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	bool s = self ? contact->colliding == self : true;
	bool o = other ? contact->collided == other : true;
	return s && o;
}

bool nContactHasElem(sContact* contact, sElement* element) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return (contact->colliding == element || contact->collided == element);
}


sElement* nContactGetOpposite(sContact* contact, sElement* self) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	nUtilAssertArgument(contact->colliding == self || contact->collided == self);
	return (contact->colliding == self ? contact->collided : contact->colliding);
}

bool nContactHasDirection(sContact* contact, Uint32 direction) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return contact->direction & direction;
}

Uint32 nContactDirection(sContact* contact) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return contact->direction;
}

bool nContactHasRelativeDirection(sContact* contact, sElement* elem, Uint32 direction) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	Uint32 relativeDirection = nContactRelativeDirection(contact, elem);
	return direction & relativeDirection;
}

Uint32 nContactRelativeDirection(sContact* contact, sElement* elem) {
	
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	
	if (elem == contact->colliding) {
		return contact->direction;
	}
	if (contact->direction == nContact_UP) {
		return nContact_DOWN;
	}
	if (contact->direction == nContact_RIGHT) {
		return nContact_LEFT;
	}
	if (contact->direction == nContact_DOWN) {
		return nContact_UP;
	}
	if (contact->direction == nContact_LEFT) {
		return nContact_RIGHT;
	}
	return 0;
}

bool nContactWasAllowed(sContact* contact) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	return contact->prevented;
}

void nContactAllowCollision(sContact* contact) {
	nUtilAssertNullPointer(contact);
	nUtilAssertHash(contact->hash == nUtil_HASH_CONTACT);
	contact->prevented = true;
}


void nContactOneWayPlatformCbk(sEvent* e) {

	if(!nContactHasDirection(e->contact, nContact_DOWN)){
		nContactAllowCollision(e->contact);
	}
}

static inline bool contactIsEqual(sContact* lhs, sContact* rhs) {
	return (
		lhs->colliding == rhs->colliding && 
		lhs->collided == rhs->collided && 
		lhs->direction == rhs->direction
	);
}



