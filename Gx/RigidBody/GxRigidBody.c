#include "../Utilities/Util.h"
#include "../Private/sElement.h"
#include "../RigidBody/sElemBody.h"
#include "../Physics/GxPhysics.h"
#include "../Array/Array.h"
#include "../List/GxList.h"
#include "../Graphics/GxGraphics.h"
#include "../Scene/GxScene.h"
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

	//Flags used by GxPhysics to control if a body can move
	bool mcflag; // movement contact flag
	bool movflag; // movement flag

	//Flags used by GxPhysics trees to identify repeated elements
	uint32_t fflag; // fixed tree flag
	uint32_t dflag; // dynamic tree flag

	//Flag used by Body to see if a element is on ground
	int groundFlag;

	//contacts
	GxList* contacts;
	sArray* temp;
} sElemBody;

sElemBody* GxCreateRigidBody_(sElement* elem, const sIni* ini) {

	if(ini->body != GxElemFixed && ini->body != GxElemDynamic){
		nUtil->assertArgument(ini->body == GxElemNone);
		return NULL;
	}

	sElemBody* self = calloc(1, sizeof(sElemBody));
	nUtil->assertAlloc(self);
	self->type = ini->body == GxElemFixed ? GxElemFixed : GxElemDynamic;
	elem->body = self;
	self->cmask = self->cmask == GxElemDynamic ? GxCmaskDynamic : GxCmaskFixed;	
	self->velocity.x = ini->velocity.x;
	self->velocity.y = ini->velocity.y;
	self->elasticity = 0.0;
	self->restitution = 1.0;
	self->friction = ini->friction ? ini->friction : false;
	self->preference = self->type == GxElemDynamic ? 1 : INT_MAX;
	self->maxgvel = self->type == GxElemDynamic? -20 : 0;		
	self->mcflag = false;
	self->movflag = false;
	self->dflag = 0;
	self->fflag = 0;
	self->contacts = GxCreateList();
	self->temp = nArr->create();
	self->groundFlag = 0;
	return self;
}

void GxDestroyRigidBody_(sElemBody* self) {
	if (self) {
		nArr->destroy(self->temp);
		GxDestroyList(self->contacts);
		free(self);
	}
}

bool GxElemHasDynamicBody(sElement* self) {
	validateElem(self, false, false);
	return self->body && self->body->type == GxElemDynamic;
}

bool GxElemHasFixedBody(sElement* self) {
	validateElem(self, false, false);
	return self->body && self->body->type == GxElemFixed;
}

bool GxElemIsOnGround(sElement* self) {
	validateElem(self, true, false);
	return self->body->groundFlag;
}

Uint32 GxElemGetCmask(sElement* self) {
	validateElem(self, true, false);
	return self->body->cmask;
}

void GxElemSetCmask(sElement* self, Uint32 mask) {
	validateElem(self, true, false);
	self->body->cmask = mask;
}

int GxElemGetPreference(sElement* self) {
	validateElem(self, true, false);
	return self->body->preference;
}

void GxElemSetPreference(sElement* self, int value) {
	validateElem(self, true, false);
	self->body->preference = value;
}

bool GxElemHasFriction(sElement* self) {
	validateElem(self, true, false);
	return self->body->friction;
}

void GxElemSetFriction(sElement* self, bool value) {
	validateElem(self, true, false);
	self->body->friction = value;
}

sVector GxElemGetVelocity(sElement* self) {
	validateElem(self, true, false);
	const GxVelocity* svel = &self->body->velocity;
	sVector vel = {
		.x = svel->x > 0 ? (int) (svel->x + 0.5) : (int)(svel->x - 0.5),
		.y = svel->y > 0 ? (int) (svel->y + 0.5) : (int)(svel->y - 0.5)
	};
	return vel;
}


void GxElemSetVelocity(sElement* self, sVector velocity){
	validateElem(self, true, false);	
	self->body->velocity.x = velocity.x;
	self->body->velocity.y = velocity.y;
}

int GxElemGetVely(sElement* self) {
	validateElem(self, true, false);
	return self->body->velocity.y > 0 ?
		(int) (self->body->velocity.y + 0.5) : (int) (self->body->velocity.y - 0.5);
}

void GxElemSetVely(sElement* self, int y) {
	validateElem(self, true, false);
	self->body->velocity.y = y;
}

int GxElemGetVelx(sElement* self) {
	validateElem(self, true, false);
	return self->body->velocity.x > 0 ?
		(int) (self->body->velocity.x + 0.5) : (int) (self->body->velocity.x - 0.5);
}

void GxElemSetVelx(sElement* self, int x) {
	validateElem(self, true, false);
	self->body->velocity.x = x;}

void GxElemAccelerate(sElement* self, double x, double y) {
	validateElem(self, true, false);
	self->body->velocity.x += x;
	self->body->velocity.y += y;
}

bool GxElemIsMoving(sElement* self) {
	validateElem(self, true, false);
	sVector velocity = GxElemGetVelocity(self);
	return (velocity.x || velocity.y);
}

double GxElemGetElasticity(sElement* self) {
	validateElem(self, true, false);
	return self->body->elasticity;
}

void GxElemSetElasticity(sElement* self, double elasticity) {
	validateElem(self, true, false);
	self->body->elasticity = elasticity;
}

double GxElemGetRestitution(sElement* self) {
	validateElem(self, true, false);
	return self->body->restitution;
}

void GxElemSetRestitution(sElement* self, double restitution) {
	validateElem(self, true, false);
	self->body->restitution = restitution;
}

int GxElemGetMaxgvel(sElement* self) {
	validateElem(self, true, false);
	return self->body->maxgvel;
}

void GxElemSetMaxgvel(sElement* self, int value) {
	validateElem(self, true, false);
	self->body->maxgvel = value;
	if (self->body->maxgvel > 0) self->body->maxgvel *= -1;
}

sArray* GxElemGetContacts(sElement* self, int direction) {
	validateElem(self, true, false);

	nArr->clean(self->body->temp);

	for(GxContact* contact = GxListBegin(self->body->contacts); contact != NULL;
		contact = GxListNext(self->body->contacts))
	{
		if (GxContactHasDirection(contact, direction)) {
			nArr->push(self->body->temp, contact, NULL);
		}
	}
	return self->body->temp;
}

GxList* GxElemGetContactList_(sElement* self) {
	validateElem(self, true, false);
	return self->body->contacts;
}

void elemRemoveContact_(sElement* self, GxContact* contact) {
	validateElem(self, true, false);

	//first remove contact
	GxListRemove(self->body->contacts, contact);

	//then change ground flag if contact is down and not prevented
	if (GxContactIsElemDownContact(contact, self) && !GxContactIsPrevented(contact)){
		--self->body->groundFlag;
	}
}


void elemAddContact_(sElement* self, GxContact* contact) {
	validateElem(self, true, false);

	//fist add contact
	GxListPush(self->body->contacts, contact, NULL);

	//then change ground flag if contact is down and not prevented
	if (GxContactIsElemDownContact(contact, self) && !GxContactIsPrevented(contact)){
		self->body->groundFlag++;
	}	
}

uint32_t GxElemGetDFlag_(sElement* self) {
	validateElem(self, true, false);
	return self->body->dflag;
}

void GxElemSetDFlag_(sElement* self, uint32_t value) {
	validateElem(self, true, false);
	self->body->dflag = value;
}
uint32_t GxElemGetFFlag_(sElement* self) {
	validateElem(self, true, false);
	return self->body->fflag;
}

void GxElemSetFFlag_(sElement* self, uint32_t value) {
	validateElem(self, true, false);
	self->body->fflag = value;
}

bool GxElemGetMcFlag_(sElement* self) {
	validateElem(self, true, false);
	return self->body->mcflag;
}

void GxElemSetMcFlag_(sElement* self, bool value) {
	validateElem(self, true, false);
	self->body->mcflag = value;
}

bool GxElemGetMovFlag_(sElement* self) {
	validateElem(self, true, false);
	return self->body->movflag;
}

void GxElemSetMovFlag_(sElement* self, bool value) {
	validateElem(self, true, false);
	self->body->movflag = value;
}

sVector GxElemMove(sElement* self, sVector vector, bool force) {
	validateElem(self, false, false);
	if (vector.x == 0 && vector.y == 0) {
		return vector;
	}
	if (self->body) {
		Uint32 mask = self->body->cmask;
		GxVelocity velocity = self->body->velocity;
		int gvel = self->body->maxgvel;
		self->body->cmask = force ? 1u << 31 : mask;
		self->body->velocity.x = vector.x;
		self->body->velocity.y = vector.y;
		self->body->maxgvel = 0;
		sVector GxPhysicsMoveCalledByElem_(GxPhysics * self, sElement * element);
		vector = GxPhysicsMoveCalledByElem_(GxSceneGetPhysics(self->scene), self);
		self->body->cmask = mask;
		self->body->velocity = velocity;
		self->body->maxgvel = gvel;
	}
	else {
		GxElemExecuteMove_(self, vector);
	}
	return vector;
}

void GxElemMoveTo(sElement* self, sPoint pos, bool force) {
	validateElem(self, false, false);
	GxElemMove(self, (sVector){ pos.x - self->pos->x, pos.y - self->pos->y }, force);
}

void GxElemExecuteMove_(sElement* self, sVector vector) {
	validateElem(self, false, false);
	SDL_Rect previousPos = *self->pos;
	self->pos->x += vector.x;
	self->pos->y += vector.y;
	GxGraphicsUpdatePosition_(GxSceneGetGraphics(self->scene), self, previousPos);
	GxPhysicsUpdateElementPosition_(GxSceneGetPhysics(self->scene), self, previousPos);
}

void GxElemApplyHozElasticity_(sElement* self, double res) {
	self->body->velocity.x *= -(self->body->elasticity * res);
}

void GxElemApplyVetElasticity_(sElement* self, double res) {
	self->body->velocity.y *= -(self->body->elasticity * res);
}


