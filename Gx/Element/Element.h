#ifndef GX_ELEM_H
#define GX_ELEM_H
#include "../Utilities/Util.h"
#include "../Ini/Ini.h"
#include "../App/App.h"
#include "../Event/GxEvent.h"
#include <stdint.h>

//... forward declarations
typedef struct sElement sElement;
struct sElemBody;
struct sElemRenderable;
struct sElemBodyNamespace;
struct sElemRenderableNamespace;

//... namespace structs
typedef struct sElemNamespace {
	
	sElement* (*create)(const sIni* ini);
	
	void (*remove)(sElement* self);
	void* (*target)(sElement* self);
		
	Uint32 (*id)(sElement* self);
	const char* (*className)(sElement* self);
	bool (*hasHandler)(sElement* self, int type);
	GxHandler (*getHandler)(sElement* self, int type);	

	bool (*hasClass)(sElement* self, const char* type);	

	GxScene* (*scene)(sElement* self);
	const SDL_Rect* (*position)(sElement* self);
	void (*setPosition)(sElement* self, SDL_Rect pos);
	sPoint (*calcCenter)(sElement* self);

	bool (*hasBody)(sElement* self);
	bool (*isRenderable)(sElement* self);

	const struct sElemBodyNamespace* body;
	const struct sElemRenderableNamespace* style;

	struct sElemPrivateNamespace {
		void (*destroy)(sElement* self);
		Uint32 (*id)(sElement* self);
		void (*executeContactHandler)(sElement* self, int type, GxContact* contact);
		struct sElemBody* (*body)(sElement* self);
		struct sElemRenderable*(*renderable)(sElement* self);
		void (*setBody)(sElement* self, struct sElemBody* body);
		void (*setRenderable)(sElement* self, struct sElemRenderable* renderable);
		void (*updatePosition)(sElement* self, sVector vector);
	}* p;
} sElemNamespace;


struct sElemBodyNamespace {

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

	struct sElemBodyPrivateNamespace {
		struct sElemBody* (*create)(sElement* elem, const sIni* ini);
		void (*destroy)(struct sElemBody* self);	

		void (*applyHozElasticity)(sElement* self, double res);
		void (*applyVetElasticity)(sElement* self, double res);

		GxList* (*getContactList)(sElement* self);

		void (*addContact)(sElement * self, GxContact * contact);
		void (*removeContact)(sElement * self, GxContact * contact);

		uint32_t (*dFFlag)(sElement* self);
		void (*setDFlag)(sElement* self, uint32_t value);

		uint32_t (*fFlag)(sElement* self);
		void (*setFFlag)(sElement* self, uint32_t value);

		bool (*mcFlag)(sElement* self);
		void (*setMcFlag)(sElement* self, bool value);

		bool (*movFlag)(sElement* self);
		void (*setMovFlag)(sElement* self, bool value);
	}* p;
};


struct sElemRenderableNamespace {

	bool (*hasRelativePosition)(sElement* self);
	bool (*hasAbsolutePosition)(sElement* self);

	int (*zIndex)(sElement* self);
	void (*setZIndex)(sElement* self, int index);

	Uint8 (*opacity)(sElement* self);
	void (*setOpacity)(sElement* self, Uint8 value);

	int (*orientation)(sElement* self);
	void (*setOrientation)(sElement* self, int value);

	const char* (*image)(sElement* self);
	void (*setImage)(sElement* self, const char* apath);	

	const char* (*animation)(sElement* self);
	void (*setAnimation)(sElement* self, const char* apath);

	const char* (*alignment)(sElement* self);
	void (*setAlignment)(sElement* self, const char* value);

	bool (*isHidden)(sElement* self);
	void (*hide)(sElement* self);
	void (*show)(sElement* self);

	double (*angle)(sElement* self);
	void (*setAngle)(sElement* self, double angle);

	double (*proportion)(sElement* self);
	void (*setProportion)(sElement* self, double proportion);
	void (*setToFit)(sElement* self, const char* axis);

	const SDL_Color* (*backgroundColor)(sElement* self);
	void (*setBackgroundColor)(sElement* self, const char* color);

	int (*borderSize)(sElement* self);
	const SDL_Color*(*borderColor)(sElement* self);
	void (*setBorder)(sElement* self, const char* border);

	const char* (*text)(sElement* self);
	void (*setText)(sElement* self, const char* text, ...);
	
	int (*fontSize)(sElement* self);
	void (*setFontSize)(sElement* self, int size);	

	void (*setFont)(sElement* self, const char* font);
	const char* (*font)(sElement* self);

	const SDL_Color* (*color)(sElement* self);
	void (*setColor)(sElement* self, const char* color);
	
	struct sElemRenderablePrivateNamespace {
		struct sElemRenderable* (*create)(sElement* elem, const sIni* ini);
		void (*destroy)(struct sElemRenderable* self);
		void (*updateLabel)(sElement* self);

		uint32_t (*wFlag)(sElement* self);
		void (*setWFlag)(sElement* self, uint32_t value);
	}* p;	
};

extern const sElemNamespace* nElem;
extern const struct sElemBodyNamespace nBodyNamespaceInstance;
extern const struct sElemRenderableNamespace nRenderableNamespaceInstance;

#endif // !GX_ELEM_H
