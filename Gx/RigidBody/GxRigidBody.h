#ifndef GX_RIGID_BODY_H
#define GX_RIGID_BODY_H
#include "../Utilities/Util.h"
#include "../sElement/sElement.h"

bool GxElemIsOnGround(sElement* self);

Uint32 GxElemGetCmask(sElement* self);
void GxElemSetCmask(sElement* self, Uint32 mask);

int GxElemGetPreference(sElement* self);
void GxElemSetPreference(sElement* self, int value);

bool GxElemHasFriction(sElement* self);
void GxElemSetFriction(sElement* self, bool value);

sVector GxElemGetVelocity(sElement* self);
void GxElemSetVelocity(sElement* self, sVector velocity);

int GxElemGetVely(sElement* self);
void GxElemSetVely(sElement* self, int vel);

int GxElemGetVelx(sElement* self);
void GxElemSetVelx(sElement* self, int vel);

void GxElemAccelerate(sElement* self, double x, double y);

bool GxElemIsMoving(sElement* self);

double GxElemGetElasticity(sElement* self);
void GxElemSetElasticity(sElement* self, double elasticity);

double GxElemGetRestitution(sElement* self);
void GxElemSetRestitution(sElement* self, double restitution);

int GxElemGetMaxgvel(sElement* self);
void GxElemSetMaxgvel(sElement* self, int value);

sArray* GxElemGetContacts(sElement* self, int types);
GxList* GxElemGetContactList_(sElement* self);

void elemAddContact_(sElement * self, GxContact * contact);
void elemRemoveContact_(sElement * self, GxContact * contact);

uint32_t GxElemGetDFlag_(sElement* self);
void GxElemSetDFlag_(sElement* self, uint32_t value);

uint32_t GxElemGetFFlag_(sElement* self);
void GxElemSetFFlag_(sElement* self, uint32_t value);

bool GxElemGetMcFlag_(sElement* self);
void GxElemSetMcFlag_(sElement* self, bool value);

bool GxElemGetMovFlag_(sElement* self);
void GxElemSetMovFlag_(sElement* self, bool value);

sVector GxElemMove(sElement* self, sVector vector, bool force);
void GxElemMoveTo(sElement* self, sPoint pos, bool force);
void GxElemExecuteMove_(sElement* self, sVector vector);

void GxElemApplyHozElasticity_(sElement* self, double res);
void GxElemApplyVetElasticity_(sElement* self, double res);


#endif // !RIGID_BODY_H

