#ifndef GX_PHYSICS_H
#define GX_PHYSICS_H
#include "../Utilities/Util.h"

extern const struct sPhysicsNamespace {
	//...
	sPhysics* (*create)(sScene* scene);
	void (*destroy)(sPhysics* self);

	//...
	void (*update)(sPhysics* self);
	void (*insert)(sPhysics* self, sElement* element);
	void (*remove)(sPhysics* self, sElement* element);
	void (*updateElem)(sPhysics* self, sElement* element, SDL_Rect previousPos);
	sVector (*moveByElem)(sPhysics* self, sElement* element);
	void (*createWalls)(sPhysics* self);
}* nPhysics;

extern const struct sContactNamespace {		
	sElement* (*colliding)(sContact* contact);
	sElement* (*collided)(sContact* contact);
	bool (*isBetween)(sContact* contact, sElement* self, sElement* other);
	bool (*hasElem)(sContact* contact, sElement* element);
	bool (*hasDirection)(sContact* contact, Uint32 direction);	
	Uint32 (*direction)(sContact* contact);
	sElement* (*getOpposite)(sContact* contact, sElement* self);
	bool (*wasAllowed)(sContact* contact);
	void (*allowCollision)(sContact* contact);
	bool (*isFromRight)(sContact* contact, sElement* self);
	bool (*isFromLeft)(sContact* contact, sElement* self);
	bool (*isFromBelow)(sContact* contact, sElement* self);
	bool (*isFromAbove)(sContact* contact, sElement* self);
	void (*oneWayPlatformCbk)(GxEvent* e);
	const Uint32 RIGHT;
	const Uint32 LEFT;
	const Uint32 HORIZONTAL;
	const Uint32 UP;
	const Uint32 DOWN;
	const Uint32 VERTICAL;
	const Uint32 ALL;
}* nContact;


#endif // !GX_PHYSICS_H
