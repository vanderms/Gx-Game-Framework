#ifndef GX_BODY_H
#define GX_BODY_H
#include "../../Util/Util.h"

struct sElemBody;

extern const struct sElemBodyNamespace {

	bool (*isDynamic)(sElement* self);
	bool (*isFixed)(sElement* self);

	bool (*isOnGround)(sElement* self);
	Uint32 (*cmask)(sElement* self);
	void (*setCmask)(sElement* self, Uint32 mask);

	int (*preference)(sElement* self);
	void (*setPreference)(sElement* self, int value);

	bool (*hasFriction)(sElement* self);
	void (*setFriction)(sElement* self, bool value);

	sVector (*velocity)(sElement* self);
	void (*setVelocity)(sElement* self, sVector velocity);

	int (*vely)(sElement* self);
	void (*setVely)(sElement* self, int vel);

	int (*velx)(sElement* self);
	void (*setVelx)(sElement* self, int vel);

	void (*accelerate)(sElement* self, double x, double y);

	bool (*isMoving)(sElement* self);

	double (*elasticity)(sElement* self);
	void (*setElasticity)(sElement* self, double elasticity);

	double (*restitution)(sElement* self);
	void (*setRestitution)(sElement* self, double restitution);

	int (*maxgvel)(sElement* self);
	void (*setMaxgvel)(sElement* self, int value);

	sArray* (*getContacts)(sElement* self, int types);

	sVector (*move)(sElement* self, sVector vector, bool force);
	void (*moveTo)(sElement* self, sPoint pos, bool force);

	const int NONE;
	const int FIXED;
	const int DYNAMIC;	
	const Uint32 CMASK_NONE;
	const Uint32 CMASK_ALL;
	const Uint32 CMASK_CAMERA;
	const Uint32 CMASK_DYNAMIC;
	const Uint32 CMASK_FIXED;

	struct sElemBodyPrivateNamespace {
		struct sElemBody* (*create)(sElement* elem, const sIni* ini);
		void (*destroy)(struct sElemBody* self);	

		void (*applyHozElasticity)(sElement* self, double res);
		void (*applyVetElasticity)(sElement* self, double res);

		sList* (*getContactList)(sElement* self);

		void (*addContact)(sElement * self, sContact * contact);
		void (*removeContact)(sElement * self, sContact * contact);

		sQtreeElem* (*getQtreeElemFixed)(sElement* self);
		sQtreeElem* (*getQtreeElemDynamic)(sElement* self);

		bool (*mcFlag)(sElement* self);
		void (*setMcFlag)(sElement* self, bool value);

		bool (*movFlag)(sElement* self);
		void (*setMovFlag)(sElement* self, bool value);
	}* p;
} nElemBody;








#endif // !GX_BODY_H
