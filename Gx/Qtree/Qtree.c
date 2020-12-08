#include "../Util/Util.h"
#include "../Qtree/Qtree.h"
#include "../List/List.h"
#include "../Array/Array.h"
#include <stdint.h>
#include <string.h>
#define MAX_ELEMENTS 16
#define MIN_LENGTH 64


#define hasIntersection(a, b)\
(!((a)->x >= (b)->x + (b)->w || (a)->x + (a)->w <= (b)->x ||\
(a)->y >= (b)->y + (b)->h || (a)->y + (a)->h <= (b)->y))

typedef struct sQtree {
	sQtree* parent;
	Uint32* counter;
	sRect pos;
	sArray* children;
	sList* elements;
} sQtree;


typedef struct sQtreeElem {
	void* elem;
	const sRect* (*posGetter)(void* elem);
	Uint32 flag;
} sQtreeElem;

sQtreeElem* createQtreeElem(void* elem, const sRect* (*posGetter)(void* elem)) {
	sQtreeElem* self = nUtil->assertAlloc(malloc(sizeof(sQtreeElem)));
	self->elem = elem;
	self->posGetter = posGetter;
	return self;
}

void destroyQtreeElem(sQtreeElem* self) {
	if (self) {
		free(self);
	}
}

void* getElem(sQtreeElem* self){
	return self->elem;
};

static void subdivide(sQtree* self);

static sQtree* create(sQtree* parent, sRect pos){		
	
	sQtree* self = malloc(sizeof(sQtree));
	nUtil->assertAlloc(self);
	self->parent = parent;
	self->counter = parent ? parent->counter : nUtil->createUint(0);
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
static void insert(sQtree* self, sQtreeElem* element) {

	const sRect* elemPos = element->posGetter(element->elem);
		
	if (hasIntersection(&self->pos, elemPos)) {

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

static void removeNode(sQtree* self, sQtreeElem* element) {	
	const sRect* elemPos = element->posGetter(element->elem);
	if ((!self->elements && !self->children) || 
		!hasIntersection(&self->pos, elemPos)) return;
	if (self->children) {
		for (Uint32 i = 0; i < nArray->size(self->children); i++) {
			sQtree* child = nArray->at(self->children, i);
			nQtree->remove(child, element);			
		}
	}
	else nList->remove(self->elements, element);
}

static void update(sQtree* self, sQtreeElem* element, sRect previous) {	
	
	const sRect* elemPos = element->posGetter(element->elem);	
	
	bool has = hasIntersection(&self->pos, elemPos);
	bool had = hasIntersection(&self->pos, &previous);

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
	nArray->push(self->children, create(self, qtree01), destroy);
	nArray->push(self->children, create(self, qtree02), destroy);
	nArray->push(self->children, create(self, qtree03), destroy);
	nArray->push(self->children, create(self, qtree04), destroy);

	//then transfer all entities to childrens
	for (sQtreeElem* elem = nList->begin(self->elements); elem != NULL; 
		elem = nList->next(self->elements)) 
	{
		nQtree->insert(self, elem);
	}

	//finally, delete and nullify self->elements
	nList->destroy(self->elements);
	self->elements = NULL;
}


static void getAllElementsInArea(sQtree* self, const sRect* area, sArray* arr, bool begin){
	
	if (begin) {
		*self->counter = *self->counter + 1; 
	}

	if (hasIntersection(&self->pos, area)) {

		for (sQtreeElem* elem = nList->begin(self->elements); elem != NULL; 
				elem = nList->next(self->elements)){			

			if (elem->flag != *self->counter) {
				elem->flag = *self->counter;
				const sRect* elemPos = elem->posGetter(elem->elem);	
				if(hasIntersection(elemPos, area)){									
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
	.createQtreeElem = createQtreeElem,
	.destroyQtreeElem = destroyQtreeElem,
	.getElem = getElem,
	.create = create,
	.destroy = destroy,
	.position = position,
	.insert = insert,
	.remove = removeNode,
	.update = update,
	.getAllElementsInArea = getAllElementsInArea,	
};