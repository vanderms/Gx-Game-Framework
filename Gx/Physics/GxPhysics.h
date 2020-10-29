#ifndef GX_PHYSICS_H
#define GX_PHYSICS_H
#include "../Utilities/GxUtil.h"

//constructor and destructors
GxPhysics* GxCreatePhysics_(GxScene* scene);
void GxDestroyPhysics_(GxPhysics* self);

//physics methods
void GxPhysicsUpdate_(GxPhysics* self);
void GxPhysicsInsertElement_(GxPhysics* self, GxElement* element);
void GxPhysicsRemoveElement_(GxPhysics* self, GxElement* element);
void GxPhysicsUpdateElementPosition_(GxPhysics* self, GxElement* element, SDL_Rect previousPos);
GxVector GxPhysicsMoveCalledByElem_(GxPhysics* self, GxElement* element);
void GxPhysicsCreateWalls_(GxPhysics* self);

//contact methods
GxElement* GxContactGetColliding(GxContact* contact);
GxElement* GxContactGetCollided(GxContact* contact);

bool GxContactIsBetween(GxContact* contact, GxElement* self, GxElement* other);
bool GxContactHasElement(GxContact* contact, GxElement* element);
bool GxContactHasDirection(GxContact* contact, Uint32 direction);
Uint32 GxContactGetDirection(GxContact* contact);
GxElement* GxContactGetOppositeElement(GxContact* contact, GxElement* self);
bool GxContactIsPrevented(GxContact* contact);
void GxContactAllowCollision(GxContact* contact);

bool GxContactIsElemRightContact(GxContact* contact, GxElement* self);
bool GxContactIsElemLeftContact(GxContact* contact, GxElement* self);
bool GxContactIsElemDownContact(GxContact* contact, GxElement* self);
bool GxContactIsElemUpContact(GxContact* contact, GxElement* self);

void GxContactOneWayPlatform(GxEvent* e);

#endif // !GX_PHYSICS_H
