#ifndef GX_BODY_H
#define GX_BODY_H
#include "../../Util/Util.h"

enum sElemBodyConstants {
	nELEM_BODY_NONE = 1,
	nELEM_BODY_FIXED = 2,
	nELEM_BODY_DYNAMIC = 3,
};

extern const Uint32 nELEM_CMASK_NONE;
extern const Uint32 nELEM_CMASK_ALL;
extern const Uint32 nELEM_CMASK_CAMERA;
extern const Uint32 nELEM_CMASK_DYNAMIC;
extern const Uint32 nELEM_CMASK_FIXED;

bool nElemIsBodyDynamic(sElement* self);
bool nElemIsBodyFixed(sElement* self);

bool nElemIsOnGround(sElement* self);
Uint32 nElemCmask(sElement* self);
void nElemSetCmask(sElement* self, Uint32 mask);

int nElemPreference(sElement* self);
void nElemSetPreference(sElement* self, int value);

bool nElemHasFriction(sElement* self);
void nElemSetFriction(sElement* self, bool value);

sVector nElemVelocity(sElement* self);
void nElemSetVelocity(sElement* self, sVector velocity);

int nElemVely(sElement* self);
void nElemSetVely(sElement* self, int vel);

int nElemVelx(sElement* self);
void nElemSetVelx(sElement* self, int vel);

void nElemAccelerate(sElement* self, double x, double y);

bool nElemIsMoving(sElement* self);

double nElemElasticity(sElement* self);

	//88888 LAST.......LAST... LAST....

void nElemSetElasticity(sElement* self, double elasticity);

double nElemRestitution(sElement* self);
void nElemSetRestitution(sElement* self, double restitution);

int nElemMaxgvel(sElement* self);
void nElemSetMaxgvel(sElement* self, int value);

sArray* nElemGetContacts(sElement* self, int types);

sVector nElemMove(sElement* self, sVector vector, bool force);
void nElemMoveTo(sElement* self, sPoint pos, bool force);



struct sElemBody* nElemCreateBody_(sElement* elem, const sIni* ini);
void nElemDestroyBody_(struct sElemBody* self);	

void nElemApplyHozElasticity_(sElement* self, double res);
void nElemApplyVetElasticity_(sElement* self, double res);

sList* nElemGetContactList_(sElement* self);

void nElemAddContact_(sElement * self, sContact * contact);
void nElemRemoveContact_(sElement * self, sContact * contact);

sQtreeElem* nElemGetQtreeElemFixed_(sElement* self);
sQtreeElem* nElemGetQtreeElemDynamic_(sElement* self);

bool nElemMcFlag_(sElement* self);
void nElemSetMcFlag_(sElement* self, bool value);

bool nElemMovFlag_(sElement* self);
void nElemSetMovFlag_(sElement* self, bool value);










#endif // !GX_BODY_H
