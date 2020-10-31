#ifndef GX_PRIVATE_H
#define GX_PRIVATE_H
#include "../Utilities/GxUtil.h"

//... MAIN STRUCT
typedef struct GxElement {
	
	//main data
	uint32_t hash;
	Uint32 id;	
	char* className;
	GxArray* classList;
	GxScene* scene;
	SDL_Rect* pos;
	
	//modules
	GxRenderable* renderable;
	GxRigidBody* body;

	//event handler module
	void* target;
	GxHandler* handlers;	
	GxMap* rHandlers;

	//special elements
	void* child;
	
} GxElement;


//constructor and destructors of interfaces
GxRenderable* GxCreateRenderable_(GxElement* elem, const GxIni* ini);
void GxDestroyRenderable_(GxRenderable* self);
GxRigidBody* GxCreateRigidBody_(GxElement* elem, const GxIni* ini);
void GxDestroyRigidBody_(GxRigidBody* self);


#define validateElem(self, body_, renderable_)\
{\
	GxAssertNullPointer(self);\
	uint32_t hash = *(uint32_t*) (self);\
	GxAssertInvalidHash(hash == GxHashElement_);\
	if(body_) GxAssertNotImplemented(self->body);\
	if(renderable_) GxAssertNotImplemented(self->renderable);\
}


#endif // !GX_PRIVATE_H
