#ifndef GX_PHYSICS_H
#define GX_PHYSICS_H
#include "../Util/Util.h"


extern const Uint32 nContact_RIGHT;
extern const Uint32 nContact_LEFT;
extern const Uint32 nContact_HORIZONTAL;
extern const Uint32 nContact_UP;
extern const Uint32 nContact_DOWN;
extern const Uint32 nContact_VERTICAL;
extern const Uint32 nContact_ALLOWED;
extern const Uint32 nContact_ALL;


sPhysics* nPhysicsCreate_(sScene* scene);
void  nPhysicsDestroy_(sPhysics* self);
void nPhysicsUpdate_(sPhysics* self);
void  nPhysicsInsert_(sPhysics* self, sElement* element);
void nPhysicsRemoveElem_(sPhysics* self, sElement* element);
void nPhysicsUpdateElem_(sPhysics* self, sElement* element, sRect previousPos);
sVector nPhysicsMoveByElem_(sPhysics* self, sElement* element);
void nPhysicsCreateWalls_(sPhysics* self);

	
sElement* nContactColliding(sContact* contact);
sElement* nContactCollided(sContact* contact);
bool nContactIsBetween(sContact* contact, sElement* self, sElement* other);
bool nContactHasElem(sContact* contact, sElement* element);
bool nContactHasDirection(sContact* contact, Uint32 direction);	
Uint32 nContactDirection(sContact* contact);
bool nContactHasRelativeDirection(sContact* contact, sElement* self, Uint32 direction);
Uint32 nContactRelativeDirection(sContact* contact, sElement* self);
sElement* nContactGetOpposite(sContact* contact, sElement* self);
bool nContactWasAllowed(sContact* contact);
void nContactAllowCollision(sContact* contact);
void nContactOneWayPlatformCbk(sEvent* e);
	

#endif // !GX_PHYSICS_H
