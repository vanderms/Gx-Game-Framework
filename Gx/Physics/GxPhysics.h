#ifndef GX_PHYSICS_H
#define GX_PHYSICS_H
#include "../Utilities/Util.h"

//constructor and destructors
GxPhysics* GxCreatePhysics_(sScene* scene);
void GxDestroyPhysics_(GxPhysics* self);

//physics methods
void GxPhysicsUpdate_(GxPhysics* self);
void GxPhysicsInsertElement_(GxPhysics* self, sElement* element);
void GxPhysicsRemoveElement_(GxPhysics* self, sElement* element);
void GxPhysicsUpdateElementPosition_(GxPhysics* self, sElement* element, SDL_Rect previousPos);
sVector GxPhysicsMoveCalledByElem_(GxPhysics* self, sElement* element);
void GxPhysicsCreateWalls_(GxPhysics* self);

//contact methods
sElement* GxContactGetColliding(GxContact* contact);
sElement* GxContactGetCollided(GxContact* contact);

bool GxContactIsBetween(GxContact* contact, sElement* self, sElement* other);
bool GxContactHasElement(GxContact* contact, sElement* element);
bool GxContactHasDirection(GxContact* contact, Uint32 direction);
Uint32 GxContactGetDirection(GxContact* contact);
sElement* GxContactGetOppositeElement(GxContact* contact, sElement* self);
bool GxContactIsPrevented(GxContact* contact);
void GxContactAllowCollision(GxContact* contact);

bool GxContactIsElemRightContact(GxContact* contact, sElement* self);
bool GxContactIsElemLeftContact(GxContact* contact, sElement* self);
bool GxContactIsElemDownContact(GxContact* contact, sElement* self);
bool GxContactIsElemUpContact(GxContact* contact, sElement* self);

void GxContactOneWayPlatform(GxEvent* e);

#endif // !GX_PHYSICS_H
