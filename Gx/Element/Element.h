#ifndef GX_ELEM_H
#define GX_ELEM_H
#include "../Util/Util.h"
#include "../App/App.h"
#include "Body/Body.h"
#include "Renderable/Renderable.h"
#include <stdint.h>


//... namespace structs
extern const struct sElemNamespace {
	
	sElement* (*create)(const sIni* ini);
	
	void (*remove)(sElement* self);
	void* (*target)(sElement* self);
		
	Uint32 (*id)(sElement* self);
	const char* (*className)(sElement* self);
	bool (*hasHandler)(sElement* self, int type);
	sHandler (*getHandler)(sElement* self, int type);	

	bool (*hasClass)(sElement* self, const char* type);	

	sScene* (*scene)(sElement* self);
	const sRect* (*position)(sElement* self);
	void (*setPosition)(sElement* self, sRect pos);
	sPoint (*calcCenter)(sElement* self);

	bool (*hasBody)(sElement* self);
	bool (*isRenderable)(sElement* self);	
	void (*addComponent)(sElement* self, sComponent* comp);
	void* (*getComponent)(sElement* self, const char* name);

	const struct sElemBodyNamespace* body;
	const struct sElemRenderableNamespace* style;	

	struct sElemDisplay {
		const int NONE;
		const int ABSOLUTE;
		const int RELATIVE;	
	}* display;

	struct sElemOrientation {
		const int FORWARD;
		const int BACKWARD;
	}* orientation;

	struct sElemPrivateNamespace {
		void (*destroy)(sElement* self);
		Uint32 (*id)(sElement* self);
		const sRect* (*posGetter)(void* value);
		void (*executeHandler)(sElement* self, sEvent* ev);
		
		struct sElemBody* (*body)(sElement* self);
		struct sElemRenderable*(*renderable)(sElement* self);
		void (*setBody)(sElement* self, struct sElemBody* body);
		void (*setRenderable)(sElement* self, struct sElemRenderable* renderable);
		void (*updatePosition)(sElement* self, sVector vector);
	}* p;
} *const nElem;

#endif // !GX_ELEM_H
