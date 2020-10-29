#ifndef GX_RIGID_BODY_H
#define GX_RIGID_BODY_H
#include "../Utilities/GxUtil.h"
#include "../Element/GxElement.h"

bool GxElemIsOnGround(GxElement* self);

Uint32 GxElemGetCmask(GxElement* self);
void GxElemSetCmask(GxElement* self, Uint32 mask);

int GxElemGetPreference(GxElement* self);
void GxElemSetPreference(GxElement* self, int value);

bool GxElemHasFriction(GxElement* self);
void GxElemSetFriction(GxElement* self, bool value);

GxVector GxElemGetVelocity(GxElement* self);
void GxElemSetVelocity(GxElement* self, GxVector velocity);

int GxElemGetVely(GxElement* self);
void GxElemSetVely(GxElement* self, int vel);

int GxElemGetVelx(GxElement* self);
void GxElemSetVelx(GxElement* self, int vel);

void GxElemAccelerate(GxElement* self, double x, double y);

bool GxElemIsMoving(GxElement* self);

double GxElemGetElasticity(GxElement* self);
void GxElemSetElasticity(GxElement* self, double elasticity);

double GxElemGetRestitution(GxElement* self);
void GxElemSetRestitution(GxElement* self, double restitution);

int GxElemGetMaxgvel(GxElement* self);
void GxElemSetMaxgvel(GxElement* self, int value);

GxArray* GxElemGetContacts(GxElement* self, int types);
GxList* GxElemGetContactList_(GxElement* self);

void elemAddContact_(GxElement * self, GxContact * contact);
void elemRemoveContact_(GxElement * self, GxContact * contact);

uint32_t GxElemGetDFlag_(GxElement* self);
void GxElemSetDFlag_(GxElement* self, uint32_t value);

uint32_t GxElemGetFFlag_(GxElement* self);
void GxElemSetFFlag_(GxElement* self, uint32_t value);

bool GxElemGetMcFlag_(GxElement* self);
void GxElemSetMcFlag_(GxElement* self, bool value);

bool GxElemGetMovFlag_(GxElement* self);
void GxElemSetMovFlag_(GxElement* self, bool value);

GxVector GxElemMove(GxElement* self, GxVector vector, bool force);
void GxElemMoveTo(GxElement* self, GxPoint pos, bool force);
void GxElemExecuteMove_(GxElement* self, GxVector vector);

void GxElemApplyHozElasticity_(GxElement* self, double res);
void GxElemApplyVetElasticity_(GxElement* self, double res);


#endif // !RIGID_BODY_H

