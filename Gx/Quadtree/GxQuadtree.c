#include "../Utilities/GxUtil.h"
#include "../Quadtree/GxQuadtree.h"
#include "../List/GxList.h"
#include "../Array/GxArray.h"
#include <stdint.h>
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../RigidBody/GxRigidBody.h"
#include <string.h>

//... type
typedef struct GxQtree {
	const char* type;
	GxQtree* parent;	
	SDL_Rect pos;
	GxArray* children;
	GxList* elements;
} GxQtree;

//static
static uint32_t gWCounter = 0;
static uint32_t gFCounter = 0;
static uint32_t gDCounter = 0;
static const int kMaxElements = 10;
static const int kMinLength = 100;

static const char* sGraphical = "graphical";
static const char* sDynamic = "dynamic";
static const char* sFixed = "fixed";

GxQtree* GxCreateQtree_(GxQtree* parent, SDL_Rect pos, const char* type){		
	GxQtree* self = malloc(sizeof(GxQtree));
	GxAssertAllocationFailure(self);
	self->parent = parent;
	
	if (type == sGraphical || strcmp(type, "graphical") == 0) {
		self->type = sGraphical;
	}
	else if (type == sFixed || strcmp(type, "fixed") == 0) {
		self->type = sFixed;
	}
	else if (type == sDynamic || strcmp(type, "dynamic") == 0) {
		self->type = sDynamic;
	}
	
	self->type = type;
	self->pos = pos;	
	self->children = NULL;
	self->elements = NULL;
	return self;
}

void GxDestroyQtree_(GxQtree* self) {
	if (self) {
		if (self->children) GxDestroyArray(self->children);
		if (self->elements) GxDestroyList(self->elements);
		free(self);
	}
}

//acessors and mutators
SDL_Rect GxQtreeGetPosition_(GxQtree* self) {
	return self->pos;
}

//methods
void GxQtreeInsert_(GxQtree* self, GxElement* element) {

	SDL_Rect elemPos = *GxElemGetPosition(element);
		
	if (SDL_HasIntersection(&self->pos, &elemPos)) {

		if (self->children) {
			for (Uint32 i = 0; i < GxArraySize(self->children); i++) {
				GxQtree* child = GxArrayAt(self->children, i);
				GxQtreeInsert_(child, element);
			}
		}
		else if (!self->elements) {			
			self->elements = GxCreateList();
			GxListPush(self->elements, element, NULL);				
		}
		else if (GxListSize(self->elements) < kMaxElements ||			
			(self->pos.w / 2) < kMinLength) {
			if(!GxListContains(self->elements, element)) GxListPush(self->elements, element, NULL);
		}
		else if (GxListSize(self->elements) >= kMaxElements) {			
			//first subdivide
			GxQtreeSubdivide_(self);
			//then, insert element recursively
			GxQtreeInsert_(self, element);
		}
	}	
}

void GxQtreeRemove_(GxQtree* self, GxElement* element) {	
	SDL_Rect elemPos = *GxElemGetPosition(element);
	if ((!self->elements && !self->children) || 
		!SDL_HasIntersection(&self->pos, &elemPos)) return;
	if (self->children) {
		for (Uint32 i = 0; i < GxArraySize(self->children); i++) {
			GxQtree* child = GxArrayAt(self->children, i);
			GxQtreeRemove_(child, element);			
		}
	}
	else GxListRemove(self->elements, element);
}

void GxQtreeUpdate_(GxQtree* self, GxElement* element, SDL_Rect previous) {	
	
	SDL_Rect elemPos = *GxElemGetPosition(element);	
	
	bool has = SDL_HasIntersection(&self->pos, &elemPos);
	bool had = SDL_HasIntersection(&self->pos, &previous);

	if (self->children && (had || has)) {
		for (Uint32 i = 0; i < GxArraySize(self->children); i++) {
			GxQtree* child = GxArrayAt(self->children, i);
			GxQtreeUpdate_(child, element, previous);
		}
	}
	else if (had && !has) {
		if (self->elements) GxListRemove(self->elements, element);
	}
	else if (!had && has) {
		GxQtreeInsert_(self, element);
	}	
}


void GxQtreeSubdivide_(GxQtree* self) {

	int xm = self->pos.w / 2; //middle x direction
	int ym = self->pos.h / 2; // middle y direction
	int xdif = self->pos.w % 2; // variable to adjust child size in case of odd parent size x direction
	int ydif = self->pos.h % 2; // variable to adjust child size in case of odd parent size y direction

	//caculate pos
	SDL_Rect qtree01 = { self->pos.x, self->pos.y, xm, ym };
	SDL_Rect qtree02 = { self->pos.x + xm, self->pos.y, xm + xdif, ym };
	SDL_Rect qtree03 = { self->pos.x, self->pos.y + ym, xm, ym + ydif };
	SDL_Rect qtree04 = { self->pos.x + xm, self->pos.y + ym, xm + xdif, ym + ydif };
	
	//create new qtrees and Push into children list
	self->children = GxCreateArray();
	GxArrayPush(self->children, GxCreateQtree_(self, qtree01, self->type), (GxDestructor) GxDestroyQtree_);
	GxArrayPush(self->children, GxCreateQtree_(self, qtree02, self->type), (GxDestructor) GxDestroyQtree_);
	GxArrayPush(self->children, GxCreateQtree_(self, qtree03, self->type), (GxDestructor) GxDestroyQtree_);
	GxArrayPush(self->children, GxCreateQtree_(self, qtree04, self->type), (GxDestructor) GxDestroyQtree_);

	//then transfer all entities to childrens
	for (GxElement* elem = GxListBegin(self->elements); elem != NULL; 
		elem = GxListNext(self->elements)) 
	{
		GxQtreeInsert_(self, elem);
	}

	//finally, delete and nullify self->elements
	GxDestroyList(self->elements);
	self->elements = NULL;
}

void GxQtreeIterate_(GxQtree* self, SDL_Rect area, void(*callback)(GxElement*), bool begin) {

	//I am not very sure if it works flawlessly or is just a big undefined behaviour
	//The problem is I cannot imagine everything that can happen when the qtree is 
	//subdivided in the iteration callback	

	if (begin) {
		if (self->type == sGraphical){
			gWCounter++;
		}
		else if (self->type == sFixed){
			gFCounter++;
		}
		else if (self->type == sDynamic){
			gDCounter++;
		}
	}

	if (SDL_HasIntersection(&self->pos, &area)) {

		for (GxElement* elem = GxListBegin(self->elements); elem != NULL; 
				elem = GxListNext(self->elements)){
		
			if (self->type == sGraphical && GxElemGetWFlag_(elem) != gWCounter) {
				GxElemSetWFlag_(elem, gWCounter);
				callback(elem);
			}
			else if (self->type == sFixed && GxElemGetFFlag_(elem) != gFCounter) {
				GxElemSetFFlag_(elem, gFCounter);
				callback(elem);
			}
			else if (self->type == sDynamic && GxElemGetDFlag_(elem) != gDCounter) {
				GxElemSetDFlag_(elem, gDCounter);
				callback(elem);
			}
		}

		if (self->children) {
			for (Uint32 i = 0; i < GxArraySize(self->children); i++) {
				GxQtree* child = GxArrayAt(self->children, i);
				GxQtreeIterate_(child, area, callback, false);
			}
		}
	}
}
