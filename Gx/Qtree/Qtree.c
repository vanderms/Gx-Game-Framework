#include "../Utilities/Util.h"
#include "../Qtree/Qtree.h"
#include "../List/List.h"
#include "../Array/Array.h"
#include <stdint.h>
#include "../Element/Element.h"
#include <string.h>
#define MAX_ELEMENTS 16
#define MIN_LENGTH 64


//... type
typedef struct sQtree {
	const char* type;
	sQtree* parent;
	Uint32* counter;
	sRect pos;
	sArray* children;
	sList* elements;
} sQtree;

static void subdivide(sQtree* self);

static const char* sGraphical = "graphical";
static const char* sDynamic = "dynamic";
static const char* sFixed = "fixed";

static sQtree* create(sQtree* parent, sRect pos, const char* type){		
	
	sQtree* self = malloc(sizeof(sQtree));
	nUtil->assertAlloc(self);
	self->parent = parent;
	self->counter = parent ? parent->counter : nUtil->createUint(0);	
	
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

static void destroy(sQtree* self) {
	if (self) {
		if (self->children) nArray->destroy(self->children);
		if (self->elements) nList->destroy(self->elements);
		if (!self->parent) {
			free(self->counter);
		}
		free(self);
	}
}

//acessors and mutators
static sRect position(sQtree* self) {
	return self->pos;
}

//methods
static void insert(sQtree* self, sElement* element) {

	sRect elemPos = *nElem->position(element);
		
	if (SDL_HasIntersection(&self->pos, &elemPos)) {

		if (self->children) {
			for (Uint32 i = 0; i < nArray->size(self->children); i++) {
				sQtree* child = nArray->at(self->children, i);
				nQtree->insert(child, element);
			}
		}
		else if (!self->elements) {			
			self->elements = nList->create();
			nList->push(self->elements, element, NULL);				
		}
		else if (nList->size(self->elements) < MAX_ELEMENTS ||			
			(self->pos.w / 2) < MIN_LENGTH) {
			if(!nList->contains(self->elements, element)) nList->push(self->elements, element, NULL);
		}
		else if (nList->size(self->elements) >= MAX_ELEMENTS) {			
			//first subdivide
			subdivide(self);
			//then, insert element recursively
			nQtree->insert(self, element);
		}
	}	
}

static void removeNode(sQtree* self, sElement* element) {	
	sRect elemPos = *nElem->position(element);
	if ((!self->elements && !self->children) || 
		!SDL_HasIntersection(&self->pos, &elemPos)) return;
	if (self->children) {
		for (Uint32 i = 0; i < nArray->size(self->children); i++) {
			sQtree* child = nArray->at(self->children, i);
			nQtree->remove(child, element);			
		}
	}
	else nList->remove(self->elements, element);
}

static void update(sQtree* self, sElement* element, sRect previous) {	
	
	sRect elemPos = *nElem->position(element);	
	
	bool has = SDL_HasIntersection(&self->pos, &elemPos);
	bool had = SDL_HasIntersection(&self->pos, &previous);

	if (self->children && (had || has)) {
		for (Uint32 i = 0; i < nArray->size(self->children); i++) {
			sQtree* child = nArray->at(self->children, i);
			nQtree->update(child, element, previous);
		}
	}
	else if (had && !has) {
		if (self->elements) nList->remove(self->elements, element);
	}
	else if (!had && has) {
		nQtree->insert(self, element);
	}	
}


static void subdivide(sQtree* self) {

	int xm = self->pos.w / 2; //middle x direction
	int ym = self->pos.h / 2; // middle y direction
	int xdif = self->pos.w % 2; // variable to adjust child size in case of odd parent size x direction
	int ydif = self->pos.h % 2; // variable to adjust child size in case of odd parent size y direction

	//caculate pos
	sRect qtree01 = { self->pos.x, self->pos.y, xm, ym };
	sRect qtree02 = { self->pos.x + xm, self->pos.y, xm + xdif, ym };
	sRect qtree03 = { self->pos.x, self->pos.y + ym, xm, ym + ydif };
	sRect qtree04 = { self->pos.x + xm, self->pos.y + ym, xm + xdif, ym + ydif };
	
	//create new qtrees and Push into children list
	self->children = nArray->create();
	nArray->push(self->children,nQtree->create(self, qtree01, self->type), nQtree->destroy);
	nArray->push(self->children,nQtree->create(self, qtree02, self->type), nQtree->destroy);
	nArray->push(self->children,nQtree->create(self, qtree03, self->type), nQtree->destroy);
	nArray->push(self->children,nQtree->create(self, qtree04, self->type), nQtree->destroy);

	//then transfer all entities to childrens
	for (sElement* elem = nList->begin(self->elements); elem != NULL; 
		elem = nList->next(self->elements)) 
	{
		nQtree->insert(self, elem);
	}

	//finally, delete and nullify self->elements
	nList->destroy(self->elements);
	self->elements = NULL;
}


static void getAllElementsInArea(sQtree* self, sRect area, sArray* arr, bool begin){
	
	if (begin) {
		*self->counter = *self->counter + 1; 
	}

	if (SDL_HasIntersection(&self->pos, &area)) {

		for (sElement* elem = nList->begin(self->elements); elem != NULL; 
				elem = nList->next(self->elements)){			

			if (self->type == sGraphical && nElem->style->p->wFlag(elem) != *self->counter) {
				nElem->style->p->setWFlag(elem, *self->counter);
				if(SDL_HasIntersection(nElem->position(elem), &area)){									
					nArray->push(arr, elem, NULL);				
				}
			}
			else if (self->type == sFixed && nElem->body->p->fFlag(elem) != *self->counter) {
				nElem->body->p->setFFlag(elem, *self->counter);				
				if(SDL_HasIntersection(nElem->position(elem), &area)){									
					nArray->push(arr, elem, NULL);				
				}
			}
			else if (self->type == sDynamic && nElem->body->p->dFlag(elem) != *self->counter) {
				nElem->body->p->setDFlag(elem, *self->counter);				
				if(SDL_HasIntersection(nElem->position(elem), &area)){									
					nArray->push(arr, elem, NULL);				
				}
			}			
		}

		if (self->children) {
			for (Uint32 i = 0; i < nArray->size(self->children); i++) {
				sQtree* child = nArray->at(self->children, i);
				getAllElementsInArea(child, area, arr, false);
			}
		}
	}
}


const struct sQtreeNamespace* nQtree = &(struct sQtreeNamespace){
	.create = create,
	.destroy = destroy,
	.position = position,
	.insert = insert,
	.remove = removeNode,
	.update = update,
	.getAllElementsInArea = getAllElementsInArea,
};