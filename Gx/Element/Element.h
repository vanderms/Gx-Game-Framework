#ifndef GX_ELEM_H
#define GX_ELEM_H
#include "../Util/Util.h"
#include "../App/App.h"
#include "Body/Body.h"
#include "Renderable/Renderable.h"
#include <stdint.h>

	
sElement* nElemCreate(const sIni* ini);

void nElemRemove(sElement* self);
void* nElemTarget(sElement* self);
		
Uint32 nElemId(sElement* self);
const char* nElemClassName(sElement* self);
bool nElemHasHandler(sElement* self, int type);
sHandler nElemGetHandler(sElement* self, int type);	
bool nElemHasClass(sElement* self, const char* type);	

sScene* nElemScene(sElement* self);
const sRect* nElemPosition(sElement* self);
void nElemSetPosition(sElement* self, sRect pos);
sPoint nElemCalcCenter(sElement* self);

bool nElemHasBody(sElement* self);
bool nElemIsRenderable(sElement* self);	
void nElemAddComponent(sElement* self, sComponent* comp);
void* nElemGetComponent(sElement* self, const char* name);

void nElemDestroy_(sElement* self);
Uint32 nElemPId_(sElement* self);
const sRect* nElemPosGetter_(void* value);
void nElemExecuteHandler_(sElement* self, sEvent* ev);
	
struct sElemBody* nElemBody_(sElement* self);
struct sElemRenderable* nElemRenderable_(sElement* self);
void nElemSetBody_(sElement* self, struct sElemBody* body);
void nElemSetRenderable_(sElement* self, struct sElemRenderable* renderable);
void nElemUpdatePosition_(sElement* self, sVector vector);

#endif // !GX_ELEM_H
