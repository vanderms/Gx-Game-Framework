#include "../Utilities/GxUtil.h"
#include "../Private/GxPrivate.h"
#include "../RigidBody/GxRigidBody.h"
#include "../Physics/GxPhysics.h"
#include "../Array/GxArray.h"
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

typedef struct GxRigidBody {
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

	//Flags used by GxRigidBody to avoid corner collision effect in scenes with gravity
	bool rightFlag;
	bool leftFlag;

	//contacts
	GxList* contacts;
	GxArray* temp;
} GxRigidBody;

GxRigidBody* GxCreateRigidBody_(GxElement* elem, const GxIni* ini) {

	if(!(ini->modules & (GxBodyDynamic | GxBodyFixed))){
		GxAssertInvalidArgument(ini->modules & GxBodyNone);
		return NULL;
	}

	GxRigidBody* self = calloc(1, sizeof(GxRigidBody));
	GxAssertAllocationFailure(self);
	self->type = ini->modules & GxBodyFixed ? GxBodyFixed : GxBodyDynamic;
	elem->body = self;
	self->cmask = ini->cmask ? *(ini->cmask): (
		self->cmask == GxBodyDynamic ? *(GxCmaskDynamic) : *(GxCmaskFixed)
	);
	self->velocity.x = ini->velocity.x;
	self->velocity.y = ini->velocity.y;
	self->elasticity = ini->elasticity ? *ini->elasticity : 0.0;
	self->restitution = ini->restitution? *ini->restitution : 1.0;
	self->friction = ini->friction ? ini->friction : false;
	self->preference = ini->preference ? *ini->preference : (
		self->type == GxBodyDynamic ? 1 : INT_MAX
	);
	self->maxgvel = self->type == GxBodyDynamic? -20 : 0;
	self->maxgvel = ini->maxgvel ? ini->maxgvel : self->maxgvel;
	self->maxgvel = self->maxgvel > 0 ? -self->maxgvel : self->maxgvel;
	self->mcflag = false;
	self->movflag = false;
	self->dflag = 0;
	self->fflag = 0;
	self->contacts = GxCreateList();
	self->temp = GxCreateArray();
	self->groundFlag = 0;
	return self;
}

void GxDestroyRigidBody_(GxRigidBody* self) {
	if (self) {
		GxDestroyArray(self->temp);
		GxDestroyList(self->contacts);
		free(self);
	}
}

bool GxElemHasDynamicBody(GxElement* self) {
	validateElem(self, false, false);
	return self->body && self->body->type == GxBodyDynamic;
}

bool GxElemHasFixedBody(GxElement* self) {
	validateElem(self, false, false);
	return self->body && self->body->type == GxBodyFixed;
}

bool GxElemIsOnGround(GxElement* self) {
	validateElem(self, true, false);
	return self->body->groundFlag;
}

Uint32 GxElemGetCmask(GxElement* self) {
	validateElem(self, true, false);
	return self->body->cmask;
}

void GxElemSetCmask(GxElement* self, Uint32 mask) {
	validateElem(self, true, false);
	self->body->cmask = mask;
}

int GxElemGetPreference(GxElement* self) {
	validateElem(self, true, false);
	return self->body->preference;
}

void GxElemSetPreference(GxElement* self, int value) {
	validateElem(self, true, false);
	self->body->preference = value;
}

bool GxElemHasFriction(GxElement* self) {
	validateElem(self, true, false);
	return self->body->friction;
}

void GxElemSetFriction(GxElement* self, bool value) {
	validateElem(self, true, false);
	self->body->friction = value;
}

GxVector GxElemGetVelocity(GxElement* self) {
	validateElem(self, true, false);
	const GxVelocity* svel = &self->body->velocity;
	GxVector vel = {
		.x = svel->x > 0 ? (int) (svel->x + 0.5) : (int)(svel->x - 0.5),
		.y = svel->y > 0 ? (int) (svel->y + 0.5) : (int)(svel->y - 0.5)
	};
	return vel;
}


void GxElemSetVelocity(GxElement* self, GxVector velocity){
	validateElem(self, true, false);
	if (velocity.x > 0 && !self->body->rightFlag) {
		self->body->velocity.x = velocity.x;
	}
	else if (velocity.x < 0 && !self->body->leftFlag) {
		self->body->velocity.x = velocity.x;
	}
	else if(velocity.x == 0){
		self->body->velocity.x = 0;
	}
	self->body->velocity.y = velocity.y;
}

int GxElemGetVely(GxElement* self) {
	validateElem(self, true, false);
	return self->body->velocity.y > 0 ?
		(int) (self->body->velocity.y + 0.5) : (int) (self->body->velocity.y - 0.5);
}

void GxElemSetVely(GxElement* self, int y) {
	validateElem(self, true, false);
	self->body->velocity.y = y;
}

int GxElemGetVelx(GxElement* self) {
	validateElem(self, true, false);
	return self->body->velocity.x > 0 ?
		(int) (self->body->velocity.x + 0.5) : (int) (self->body->velocity.x - 0.5);
}

void GxElemSetVelx(GxElement* self, int x) {
	validateElem(self, true, false);
	if (x > 0 && !self->body->rightFlag) {
		self->body->velocity.x = x;
	}
	else if (x < 0 && !self->body->leftFlag) {
		self->body->velocity.x = x;
	}
	else if(x == 0){
		self->body->velocity.x = 0;
	}
}

void GxElemAccelerate(GxElement* self, double x, double y) {
	validateElem(self, true, false);
	self->body->velocity.x += x;
	self->body->velocity.y += y;
}

bool GxElemIsMoving(GxElement* self) {
	validateElem(self, true, false);
	GxVector velocity = GxElemGetVelocity(self);
	return (velocity.x || velocity.y);
}

double GxElemGetElasticity(GxElement* self) {
	validateElem(self, true, false);
	return self->body->elasticity;
}

void GxElemSetElasticity(GxElement* self, double elasticity) {
	validateElem(self, true, false);
	self->body->elasticity = elasticity;
}

double GxElemGetRestitution(GxElement* self) {
	validateElem(self, true, false);
	return self->body->restitution;
}

void GxElemSetRestitution(GxElement* self, double restitution) {
	validateElem(self, true, false);
	self->body->restitution = restitution;
}

int GxElemGetMaxgvel(GxElement* self) {
	validateElem(self, true, false);
	return self->body->maxgvel;
}

void GxElemSetMaxgvel(GxElement* self, int value) {
	validateElem(self, true, false);
	self->body->maxgvel = value;
	if (self->body->maxgvel > 0) self->body->maxgvel *= -1;
}

GxArray* GxElemGetContacts(GxElement* self, int direction) {
	validateElem(self, true, false);

	GxArrayClean(self->body->temp);

	for(GxContact* contact = GxListBegin(self->body->contacts); contact != NULL;
		contact = GxListNext(self->body->contacts))
	{
		if (GxContactHasDirection(contact, direction)) {
			GxArrayPush(self->body->temp, contact, NULL);
		}
	}
	return self->body->temp;
}

GxList* GxElemGetContactList_(GxElement* self) {
	validateElem(self, true, false);
	return self->body->contacts;
}

void elemRemoveContact_(GxElement* self, GxContact* contact) {
	validateElem(self, true, false);

	//first remove contact
	GxListRemove(self->body->contacts, contact);

	//then change ground flag if contact is down and not prevented
	if (GxContactIsElemDownContact(contact, self) && !GxContactIsPrevented(contact)){
		--self->body->groundFlag;
	}
}

static void lateralFlagsTimeoutHandler(GxEvent* e) {
	GxElement* self = e->target;
	self->body->rightFlag = false;
	self->body->leftFlag = false;
}

void elemAddContact_(GxElement* self, GxContact* contact) {
	validateElem(self, true, false);

	//fist add contact
	GxListPush(self->body->contacts, contact, NULL);

	//then change ground flag if contact is down and not prevented
	if (GxContactIsElemDownContact(contact, self) && !GxContactIsPrevented(contact)){
		self->body->groundFlag++;
	}
	else if (GxContactIsElemRightContact(contact, self) && !GxContactIsPrevented(contact)){
		self->body->rightFlag = true;
		GxSceneSetTimeout(self->scene, 2, lateralFlagsTimeoutHandler, self);
	}
	if (GxContactIsElemLeftContact(contact, self) && !GxContactIsPrevented(contact)){
		self->body->leftFlag = true;
		GxSceneSetTimeout(self->scene, 2, lateralFlagsTimeoutHandler, self);
	}
}

uint32_t GxElemGetDFlag_(GxElement* self) {
	validateElem(self, true, false);
	return self->body->dflag;
}

void GxElemSetDFlag_(GxElement* self, uint32_t value) {
	validateElem(self, true, false);
	self->body->dflag = value;
}
uint32_t GxElemGetFFlag_(GxElement* self) {
	validateElem(self, true, false);
	return self->body->fflag;
}

void GxElemSetFFlag_(GxElement* self, uint32_t value) {
	validateElem(self, true, false);
	self->body->fflag = value;
}

bool GxElemGetMcFlag_(GxElement* self) {
	validateElem(self, true, false);
	return self->body->mcflag;
}

void GxElemSetMcFlag_(GxElement* self, bool value) {
	validateElem(self, true, false);
	self->body->mcflag = value;
}

bool GxElemGetMovFlag_(GxElement* self) {
	validateElem(self, true, false);
	return self->body->movflag;
}

void GxElemSetMovFlag_(GxElement* self, bool value) {
	validateElem(self, true, false);
	self->body->movflag = value;
}

GxVector GxElemMove(GxElement* self, GxVector vector, bool force) {
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
		GxVector GxPhysicsMoveCalledByElem_(GxPhysics * self, GxElement * element);
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

void GxElemMoveTo(GxElement* self, GxPoint pos, bool force) {
	validateElem(self, false, false);
	GxElemMove(self, (GxVector){ pos.x - self->pos->x, pos.y - self->pos->y }, force);
}

void GxElemExecuteMove_(GxElement* self, GxVector vector) {
	validateElem(self, false, false);
	SDL_Rect previousPos = *self->pos;
	self->pos->x += vector.x;
	self->pos->y += vector.y;
	GxGraphicsUpdatePosition_(GxSceneGetGraphics(self->scene), self, previousPos);
	GxPhysicsUpdateElementPosition_(GxSceneGetPhysics(self->scene), self, previousPos);
}

void GxElemApplyHozElasticity_(GxElement* self, double res) {
	self->body->velocity.x *= -(self->body->elasticity * res);
}

void GxElemApplyVetElasticity_(GxElement* self, double res) {
	self->body->velocity.y *= -(self->body->elasticity * res);
}


