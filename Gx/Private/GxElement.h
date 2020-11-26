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


//...RENDERABLE STRUCTS 
typedef struct Alignment{
	bool changed;
	char last[25];
	const char* horizontal;
	const char* vertical;
	int x;
	int y;
} Alignment;

typedef struct Color {
	char* last;
	SDL_Color* value;
} Color;

typedef struct Border {
	int size;
	Color* color;
} Border;

//... MAIN TYPE
typedef struct GxRenderable {
	int type;
	int zIndex;
	SDL_RendererFlip orientation;
	char* asset;
	GxArray* folders;
	Uint8 opacity;
	GxImage* image;
	GxAnimation* animation;
	uint32_t animCounter;
	Uint32 animCurrent;
	Alignment* alignment;
	bool hidden;
	double angle;
	double proportion;
	Border border;
	Color* backgroundColor;
	//label
	char* font;
	char* text;
	int fontSize;
	Color* color;
	bool shouldUpdateLabel;
	GxImage* label;

	//...
	Uint32 wflag;
} GxRenderable;

//... ALIGNMENT CONSTANTS
static const char* sRight = "right";
static const char* sLeft = "left";
static const char* sBottom = "bottom";
static const char* sTop = "top";
static const char* sCenter = "center";
static const char* sNum = "number";
static const char* sCenterCenter = "center|center";
#endif // !GX_PRIVATE_H
