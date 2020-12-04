#include "../Element.h"
#include "../../Physics/Physics.h"
#include "../../Array/Array.h"
#include "../../List/List.h"
#include "../../Graphics/Graphics.h"
#include "../../Scene/Scene.h"
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

	//Flags used by sPhysics to control if a body can move
	bool mcflag; // movement contact flag
	bool movflag; // movement flag

	//Flags used by sPhysics trees to identify repeated elements
	uint32_t fflag; // fixed tree flag
	uint32_t dflag; // dynamic tree flag

	//Flag used by Body to see if a element is on ground
	int groundFlag;

	//contacts
	sList* contacts;
	sArray* temp;
} sElemBody;

static sElemBody* pCreate(sElement* elem, const sIni* ini) {

	if(ini->body != nElem->body->FIXED && ini->body != nElem->body->DYNAMIC){
		nUtil->assertArgument(ini->body == nElem->body->NONE);
		return NULL;
	}

	sElemBody* self = calloc(1, sizeof(sElemBody));
	nUtil->assertAlloc(self);
	nElem->p->setBody(elem, self);
	self->type = (ini->body == nElem->body->FIXED ? 
		nElem->body->FIXED : nElem->body->DYNAMIC
	);		
	self->cmask = self->type == nElem->body->DYNAMIC ? GxCmaskDynamic : GxCmaskFixed;	
	self->velocity.x = ini->velocity.x;
	self->velocity.y = ini->velocity.y;
	self->elasticity = 0.0;
	self->restitution = 1.0;
	self->friction = ini->friction ? ini->friction : false;
	self->preference = self->type == nElem->body->DYNAMIC ? 1 : INT_MAX;
	self->maxgvel = self->type == nElem->body->DYNAMIC? -20 : 0;		
	self->mcflag = false;
	self->movflag = false;
	self->dflag = 0;
	self->fflag = 0;
	self->contacts = nList->create();
	self->temp = nArray->create();
	self->groundFlag = 0;
	return self;
}

static void pDestroy(sElemBody* self) {
	if (self) {
		nArray->destroy(self->temp);
		nList->destroy(self->contacts);
		free(self);
	}
}

static bool isDynamic(sElement* self) {
	sElemBody* body = nElem->p->body(self);	
	return body->type == nElem->body->DYNAMIC;
}

static bool isFixed(sElement* self) {
	sElemBody* body = nElem->p->body(self);	
	return body->type == nElem->body->FIXED;
}

static bool isOnGround(sElement* self) {
	sElemBody* body = nElem->p->body(self);	
	return body->groundFlag;
}

static Uint32 cmask(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->cmask;
}

static void setCmask(sElement* self, Uint32 mask) {
	sElemBody* body = nElem->p->body(self);
	body->cmask = mask;
}

static int preference(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->preference;
}

static void setPreference(sElement* self, int value) {
	sElemBody* body = nElem->p->body(self);
	body->preference = value;
}

static bool hasFriction(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->friction;
}

static void setFriction(sElement* self, bool value) {
	sElemBody* body = nElem->p->body(self);
	body->friction = value;
}

static sVector velocity(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	const GxVelocity* svel = &body->velocity;
	sVector vel = {
		.x = svel->x > 0 ? (int) (svel->x + 0.5) : (int)(svel->x - 0.5),
		.y = svel->y > 0 ? (int) (svel->y + 0.5) : (int)(svel->y - 0.5)
	};
	return vel;
}


static void setVelocity(sElement* self, sVector velocity){
	sElemBody* body = nElem->p->body(self);	
	body->velocity.x = velocity.x;
	body->velocity.y = velocity.y;
}

static int vely(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->velocity.y > 0 ?
		(int) (body->velocity.y + 0.5) : (int) (body->velocity.y - 0.5);
}

static void setVely(sElement* self, int y) {
	sElemBody* body = nElem->p->body(self);
	body->velocity.y = y;
}

static int velx(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->velocity.x > 0 ?
		(int) (body->velocity.x + 0.5) : (int) (body->velocity.x - 0.5);
}

static void setVelx(sElement* self, int x) {
	sElemBody* body = nElem->p->body(self);
	body->velocity.x = x;
}

static void accelerate(sElement* self, double x, double y) {
	sElemBody* body = nElem->p->body(self);
	body->velocity.x += x;
	body->velocity.y += y;
}

static bool isMoving(sElement* self) {	
	sVector velocity = nElem->body->velocity(self);
	return (velocity.x || velocity.y);
}

static double elasticity(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->elasticity;
}

static void setElasticity(sElement* self, double elasticity) {
	sElemBody* body = nElem->p->body(self);
	body->elasticity = elasticity;
}

static double restitution(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->restitution;
}

static void setRestitution(sElement* self, double restitution) {
	sElemBody* body = nElem->p->body(self);
	body->restitution = restitution;
}

static int maxgvel(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->maxgvel;
}

static void setMaxgvel(sElement* self, int value) {
	sElemBody* body = nElem->p->body(self);
	body->maxgvel = value;
	if (body->maxgvel > 0) body->maxgvel *= -1;
}

static sArray* getContacts(sElement* self, int direction) {
	sElemBody* body = nElem->p->body(self);

	nArray->clean(body->temp);

	for(sContact* contact = nList->begin(body->contacts); contact != NULL;
		contact = nList->next(body->contacts))
	{
		if (nContact->hasDirection(contact, direction)) {
			nArray->push(body->temp, contact, NULL);
		}
	}
	return body->temp;
}

static sList* pGetContactList(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->contacts;
}

static void pRemoveContact(sElement* self, sContact* contact) {
	sElemBody* body = nElem->p->body(self);

	//first remove contact
	nList->remove(body->contacts, contact);

	//then change ground flag if contact is down and not prevented
	if (nContact->isFromBelow(contact, self) && !nContact->wasAllowed(contact)){
		--body->groundFlag;
	}
}


static void pAddContact(sElement* self, sContact* contact) {
	sElemBody* body = nElem->p->body(self);

	//fist add contact
	nList->push(body->contacts, contact, NULL);

	//then change ground flag if contact is down and not prevented
	if (nContact->isFromBelow(contact, self) && !nContact->wasAllowed(contact)){
		body->groundFlag++;
	}	
}

static uint32_t pDFlag(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->dflag;
}

static void pSetDFlag(sElement* self, uint32_t value) {
	sElemBody* body = nElem->p->body(self);
	body->dflag = value;
}

static uint32_t pFFlag(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->fflag;
}

static void pSetFFlag(sElement* self, uint32_t value) {
	sElemBody* body = nElem->p->body(self);
	body->fflag = value;
}

static bool pMcFlag(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->mcflag;
}

static void pSetMcFlag(sElement* self, bool value) {
	sElemBody* body = nElem->p->body(self);
	body->mcflag = value;
}

static bool pMovFlag(sElement* self) {
	sElemBody* body = nElem->p->body(self);
	return body->movflag;
}

static void pSetMovFlag(sElement* self, bool value) {
	sElemBody* body = nElem->p->body(self);
	body->movflag = value;
}

static sVector move(sElement* self, sVector vector, bool force) {
	sElemBody* body = nElem->p->body(self);
	if (vector.x == 0 && vector.y == 0) {
		return vector;
	}	
	sScene* scene = nElem->scene(self);
	Uint32 mask = body->cmask;
	GxVelocity velocity = body->velocity;
	int gvel = body->maxgvel;
	body->cmask = force ? 1u << 31 : mask;
	body->velocity.x = vector.x;
	body->velocity.y = vector.y;
	body->maxgvel = 0;	
	vector = nPhysics->moveByElem(GxSceneGetPhysics(scene), self);
	body->cmask = mask;
	body->velocity = velocity;
	body->maxgvel = gvel;		
	return vector;
}

static void moveTo(sElement* self, sPoint pos, bool force) {	
	const SDL_Rect* selfPos = nElem->position(self);
	nElem->body->move(self, (sVector){ pos.x - selfPos->x, pos.y - selfPos->y }, force);
}

static void pApplyHozElasticity(sElement* self, double res) {
	sElemBody* body = nElem->p->body(self);
	body->velocity.x *= -(body->elasticity * res);
}

static void pApplyVetElasticity(sElement* self, double res) {
	sElemBody* body = nElem->p->body(self);
	body->velocity.y *= -(body->elasticity * res);
}

const struct sElemBodyNamespace nElemBody = {

	.isDynamic = isDynamic,
	.isFixed = isFixed,

	.isOnGround = isOnGround,
	.cmask = cmask,
	.setCmask = setCmask,

	.preference = preference,
	.setPreference = setPreference,

	.hasFriction = hasFriction,
	.setFriction = setFriction,

	.velocity = velocity,
	.setVelocity = setVelocity,

	.vely = vely,
	.setVely = setVely,

	.velx = velx,
	.setVelx = setVelx,

	.accelerate = accelerate,

	.isMoving = isMoving,

	.elasticity = elasticity,
	.setElasticity = setElasticity,

	.restitution = restitution,
	.setRestitution = setRestitution,

	.maxgvel = maxgvel,
	.setMaxgvel = setMaxgvel,

	.getContacts = getContacts,

	.move = move,
	.moveTo = moveTo,

	.NONE = 1,
	.FIXED = 2,
	.DYNAMIC = 3,	

	.p = &(struct sElemBodyPrivateNamespace) {
		.create = pCreate,
		.destroy = pDestroy,	

		.applyHozElasticity = pApplyHozElasticity,
		.applyVetElasticity = pApplyVetElasticity,

		.getContactList = pGetContactList,

		.addContact = pAddContact,
		.removeContact = pRemoveContact,

		.dFlag = pDFlag,
		.setDFlag = pSetDFlag,

		.fFlag = pFFlag,
		.setFFlag = pSetFFlag,

		.mcFlag = pMcFlag,
		.setMcFlag = pSetMcFlag,

		.movFlag = pMovFlag,
		.setMovFlag = pSetMovFlag,
	},
};
