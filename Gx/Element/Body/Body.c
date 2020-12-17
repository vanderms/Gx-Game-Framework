#include "../Element.h"
#include "../../Physics/Physics.h"
#include "../../Containers/Array/Array.h"
#include "../../Containers/List/List.h"
#include "../../Graphics/Graphics.h"
#include "../../Scene/Scene.h"
#include "../../Containers/Qtree/Qtree.h"
#include <string.h>
#include <limits.h>


//... BODY AUXILIARY TYPES
typedef enum BodyConstant {
	Up,
	Right,
	Down,
	Left
} BodyConstant;

typedef struct GxVelocity {
	double x;
	double y;
} GxVelocity;

typedef struct sElemBody {
	int type;
	Uint32 cmask;
	int preference;
	GxVelocity velocity;
	double elasticity;
	double restitution;
	bool friction;
	int maxgvel;

	//...Qtree interface
	sQtreeElem* dynamic;
	sQtreeElem* fixed;

	//Flags used by sPhysics to control if a body can move
	bool mcflag; // movement contact flag
	bool movflag; // movement flag	

	//Flag used by Body to see if a element is on ground
	int groundFlag;

	//contacts
	sList* contacts;
	sArray* temp;
} sElemBody;

const Uint32 nElem_CMASK_NONE = 0;
const Uint32 nElem_CMASK_ALL = ~0u;
const Uint32 nElem_CMASK_CAMERA = 1u << 30;
const Uint32 nElem_CMASK_DYNAMIC = 1 << 0;
const Uint32 nElem_CMASK_FIXED = (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7);


sElemBody* nElemCreateBody_(sElement* elem, const sIni* ini) {

	if(ini->body != nElem_BODY_FIXED && ini->body != nElem_BODY_DYNAMIC){
		nUtilAssertArgument(ini->body == nELEM_BODY_NONE);
		return NULL;
	}

	sElemBody* self = calloc(1, sizeof(sElemBody));
	nUtilAssertAlloc(self);
	nElemSetBody_(elem, self);
	self->type = (ini->body == nElem_BODY_FIXED ? 
		nElem_BODY_FIXED : nElem_BODY_DYNAMIC
	);		
	self->cmask = (self->type == nElem_BODY_DYNAMIC ? 
		nElem_CMASK_DYNAMIC : nElem_CMASK_FIXED
	);
	self->velocity.x = ini->velocity.x;
	self->velocity.y = ini->velocity.y;
	self->elasticity = 0.0;
	self->restitution = 1.0;
	self->friction = ini->friction ? ini->friction : false;
	self->preference = self->type == nElem_BODY_DYNAMIC ? 1 : INT_MAX;
	self->maxgvel = self->type == nElem_BODY_DYNAMIC? -20 : 0;		
	self->mcflag = false;
	self->movflag = false;
	self->dynamic = nQtreeCreateElem(elem, nElemPosGetter_);
	self->fixed = nQtreeCreateElem(elem, nElemPosGetter_);
	self->contacts = nListCreate();
	self->temp = nArrayCreate();
	self->groundFlag = 0;
	return self;
}

void nElemDestroyBody_(sElemBody* self) {
	if (self) {
		nQtreeDestroyElem(self->dynamic);
		nQtreeDestroyElem(self->fixed);
		nArrayDestroy(self->temp);
		nListDestroy(self->contacts);
		free(self);
	}
}

bool nElemIsBodyDynamic(sElement* self) {
	sElemBody* body = nElemBody_(self);	
	return body->type == nElem_BODY_DYNAMIC;
}

bool nElemIsBodyFixed(sElement* self) {
	sElemBody* body = nElemBody_(self);	
	return body->type == nElem_BODY_FIXED;
}

bool nElemIsOnGround(sElement* self) {
	sElemBody* body = nElemBody_(self);	
	return body->groundFlag;
}

Uint32 nElemCmask(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->cmask;
}

void nElemSetCmask(sElement* self, Uint32 mask) {
	sElemBody* body = nElemBody_(self);
	body->cmask = mask;
}

int nElemPreference(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->preference;
}

void nElemSetPreference(sElement* self, int value) {
	sElemBody* body = nElemBody_(self);
	body->preference = value;
}

bool nElemHasFriction(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->friction;
}

void nElemSetFriction(sElement* self, bool value) {
	sElemBody* body = nElemBody_(self);
	body->friction = value;
}

sVector nElemVelocity(sElement* self) {
	sElemBody* body = nElemBody_(self);
	const GxVelocity* svel = &body->velocity;
	sVector vel = {
		.x = svel->x > 0 ? (int) (svel->x + 0.5) : (int)(svel->x - 0.5),
		.y = svel->y > 0 ? (int) (svel->y + 0.5) : (int)(svel->y - 0.5)
	};
	return vel;
}


void nElemSetVelocity(sElement* self, sVector velocity){
	sElemBody* body = nElemBody_(self);	
	body->velocity.x = velocity.x;
	body->velocity.y = velocity.y;
}

int nElemVely(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->velocity.y > 0 ?
		(int) (body->velocity.y + 0.5) : (int) (body->velocity.y - 0.5);
}

void nElemSetVely(sElement* self, int y) {
	sElemBody* body = nElemBody_(self);
	body->velocity.y = y;
}

int nElemVelx(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->velocity.x > 0 ?
		(int) (body->velocity.x + 0.5) : (int) (body->velocity.x - 0.5);
}

void nElemSetVelx(sElement* self, int x) {
	sElemBody* body = nElemBody_(self);
	body->velocity.x = x;
}

void nElemAccelerate(sElement* self, double x, double y) {
	sElemBody* body = nElemBody_(self);
	body->velocity.x += x;
	body->velocity.y += y;
}

bool nElemIsMoving(sElement* self) {	
	sVector velocity = nElemVelocity(self);
	return (velocity.x || velocity.y);
}

double nElemElasticity(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->elasticity;
}

void nElemSetElasticity(sElement* self, double elasticity) {
	sElemBody* body = nElemBody_(self);
	body->elasticity = elasticity;
}

double nElemRestitution(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->restitution;
}


void nElemSetRestitution(sElement* self, double restitution) {
	sElemBody* body = nElemBody_(self);
	body->restitution = restitution;
}

int nElemMaxgvel(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->maxgvel;
}

void nElemSetMaxgvel(sElement* self, int value) {
	sElemBody* body = nElemBody_(self);
	body->maxgvel = value;
	if (body->maxgvel > 0) body->maxgvel *= -1;
}

sArray* nElemGetContacts(sElement* self, int direction) {
	sElemBody* body = nElemBody_(self);

	nArrayClean(body->temp);

	for(sContact* contact = nListBegin(body->contacts); contact != NULL;
		contact = nListNext(body->contacts))
	{
		if (nContactHasDirection(contact, direction)) {
			nArrayPush(body->temp, contact, NULL);
		}
	}
	return body->temp;
}

sList* nElemGetContactList_(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->contacts;
}

void nElemRemoveContact_(sElement* self, sContact* contact) {
	sElemBody* body = nElemBody_(self);

	//first remove contact
	nListRemove(body->contacts, contact);

	//then change ground flag if contact is down and not allowed
	if (nContactHasRelativeDirection(contact, self, nContact_DOWN) && 
		!nContactWasAllowed(contact)){
		--body->groundFlag;
	}
}


void nElemAddContact_(sElement* self, sContact* contact) {
	sElemBody* body = nElemBody_(self);

	//fist add contact
	nListPush(body->contacts, contact, NULL);

	//then change ground flag if contact is down and not allowed
	if (nContactHasRelativeDirection(contact, self, nContact_DOWN) && 
		!nContactWasAllowed(contact)){
		body->groundFlag++;
	}	
}

sQtreeElem* nElemGetQtreeElemFixed_(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->fixed;
}

sQtreeElem* nElemGetQtreeElemDynamic_(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->dynamic;
}

bool nElemMcFlag_(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->mcflag;
}

void nElemSetMcFlag_(sElement* self, bool value) {
	sElemBody* body = nElemBody_(self);
	body->mcflag = value;
}

bool nElemMovFlag_(sElement* self) {
	sElemBody* body = nElemBody_(self);
	return body->movflag;
}

void nElemSetMovFlag_(sElement* self, bool value) {
	sElemBody* body = nElemBody_(self);
	body->movflag = value;
}

sVector nElemMove(sElement* self, sVector vector, bool force) {	
	
	if (vector.x == 0 && vector.y == 0) {
		return vector;
	}

	if (!nElemHasBody(self)) {
		sRect pos = *nElemPosition(self);
		pos.x += vector.x;
		pos.y += vector.y;
		nElemSetPosition(self, pos);
		return vector;
	}

	sElemBody* body = nElemBody_(self);
	sScene* scene = nElemScene(self);
	Uint32 mask = body->cmask;
	GxVelocity velocity = body->velocity;
	int gvel = body->maxgvel;
	body->cmask = force ? 1u << 31 : mask;
	body->velocity.x = vector.x;
	body->velocity.y = vector.y;
	body->maxgvel = 0;	
	vector = nPhysicsMoveByElem_(nSceneGetPhysics_(scene), self);
	body->cmask = mask;
	body->velocity = velocity;
	body->maxgvel = gvel;		
	return vector;
}

void nElemMoveTo(sElement* self, sPoint pos, bool force) {	
	const sRect* selfPos = nElemPosition(self);
	nElemMove(self, (sVector){ pos.x - selfPos->x, pos.y - selfPos->y }, force);
}

void nElemApplyHozElasticity_(sElement* self, double res) {
	sElemBody* body = nElemBody_(self);
	body->velocity.x *= -(body->elasticity * res);
}

void nElemApplyVetElasticity_(sElement* self, double res) {
	sElemBody* body = nElemBody_(self);
	body->velocity.y *= -(body->elasticity * res);
}
