#include "../Utilities/Util.h"
#include "../Quadtree/GxQuadtree.h"
#include "../List/List.h"
#include "../Array/Array.h"
#include <stdint.h>
#include "../Element/Element.h"
#include <string.h>

//... type
typedef struct GxQtree {
	const char* type;
	GxQtree* parent;	
	SDL_Rect pos;
	sArray* children;
	sList* elements;
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
	nUtil->assertAlloc(self);
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
		if (self->children) nArray->destroy(self->children);
		if (self->elements) nList->destroy(self->elements);
		free(self);
	}
}

//acessors and mutators
SDL_Rect GxQtreeGetPosition_(GxQtree* self) {
	return self->pos;
}

//methods
void GxQtreeInsert_(GxQtree* self, sElement* element) {

	SDL_Rect elemPos = *nElem->position(element);
		
	if (SDL_HasIntersection(&self->pos, &elemPos)) {

		if (self->children) {
			for (Uint32 i = 0; i < nArray->size(self->children); i++) {
				GxQtree* child = nArray->at(self->children, i);
				GxQtreeInsert_(child, element);
			}
		}
		else if (!self->elements) {			
			self->elements = nList->create();
			nList->push(self->elements, element, NULL);				
		}
		else if (nList->size(self->elements) < kMaxElements ||			
			(self->pos.w / 2) < kMinLength) {
			if(!nList->contains(self->elements, element)) nList->push(self->elements, element, NULL);
		}
		else if (nList->size(self->elements) >= kMaxElements) {			
			//first subdivide
			GxQtreeSubdivide_(self);
			//then, insert element recursively
			GxQtreeInsert_(self, element);
		}
	}	
}

void GxQtreeRemove_(GxQtree* self, sElement* element) {	
	SDL_Rect elemPos = *nElem->position(element);
	if ((!self->elements && !self->children) || 
		!SDL_HasIntersection(&self->pos, &elemPos)) return;
	if (self->children) {
		for (Uint32 i = 0; i < nArray->size(self->children); i++) {
			GxQtree* child = nArray->at(self->children, i);
			GxQtreeRemove_(child, element);			
		}
	}
	else nList->remove(self->elements, element);
}

void GxQtreeUpdate_(GxQtree* self, sElement* element, SDL_Rect previous) {	
	
	SDL_Rect elemPos = *nElem->position(element);	
	
	bool has = SDL_HasIntersection(&self->pos, &elemPos);
	bool had = SDL_HasIntersection(&self->pos, &previous);

	if (self->children && (had || has)) {
		for (Uint32 i = 0; i < nArray->size(self->children); i++) {
			GxQtree* child = nArray->at(self->children, i);
			GxQtreeUpdate_(child, element, previous);
		}
	}
	else if (had && !has) {
		if (self->elements) nList->remove(self->elements, element);
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
	self->children = nArray->create();
	nArray->push(self->children, GxCreateQtree_(self, qtree01, self->type), (sDtor) GxDestroyQtree_);
	nArray->push(self->children, GxCreateQtree_(self, qtree02, self->type), (sDtor) GxDestroyQtree_);
	nArray->push(self->children, GxCreateQtree_(self, qtree03, self->type), (sDtor) GxDestroyQtree_);
	nArray->push(self->children, GxCreateQtree_(self, qtree04, self->type), (sDtor) GxDestroyQtree_);

	//then transfer all entities to childrens
	for (sElement* elem = nList->begin(self->elements); elem != NULL; 
		elem = nList->next(self->elements)) 
	{
		GxQtreeInsert_(self, elem);
	}

	//finally, delete and nullify self->elements
	nList->destroy(self->elements);
	self->elements = NULL;
}

void GxQtreeIterate_(GxQtree* self, SDL_Rect area, void(*callback)(sElement*), bool begin) {

	//I am not very sure if it works flawlessly or is just a big undefined behaviour
	//The problem is I cannot imagine everything that can happen when the qtree is 
	//subdivided in the iteration callback... and I don't know how to test it either :)

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

		for (sElement* elem = nList->begin(self->elements); elem != NULL; 
				elem = nList->next(self->elements)){
		
			if (self->type == sGraphical && nElem->style->p->wFlag(elem) != gWCounter) {
				nElem->style->p->setWFlag(elem, gWCounter);
				callback(elem);
			}
			else if (self->type == sFixed && nElem->body->p->fFlag(elem) != gFCounter) {
				nElem->body->p->setFFlag(elem, gFCounter);
				callback(elem);
			}
			else if (self->type == sDynamic && nElem->body->p->dFlag(elem) != gDCounter) {
				nElem->body->p->setDFlag(elem, gDCounter);
				callback(elem);
			}
		}

		if (self->children) {
			for (Uint32 i = 0; i < nArray->size(self->children); i++) {
				GxQtree* child = nArray->at(self->children, i);
				GxQtreeIterate_(child, area, callback, false);
			}
		}
	}
}
