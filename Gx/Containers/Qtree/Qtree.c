#include "Qtree.h"
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

sQtreeElem* nQtreeCreateElem(void* elem, const sRect* (*posGetter)(void* elem)) {
	sQtreeElem* self = nUtil->assertAlloc(malloc(sizeof(sQtreeElem)));
	self->elem = elem;
	self->posGetter = posGetter;
	return self;
}

void nQtreeDestroyElem(sQtreeElem* self) {
	if (self) {
		free(self);
	}
}

void* nQtreeGetElem(sQtreeElem* self){
	return self->elem;
};

static void subdivide(sQtree* self);

sQtree* nQtreeCreate(sQtree* parent, sRect pos){		
	
	sQtree* self = malloc(sizeof(sQtree));
	nUtil->assertAlloc(self);
	self->parent = parent;
	self->counter = parent ? parent->counter : nUtil->createUint(0);
	self->pos = pos;	
	self->children = NULL;
	self->elements = NULL;
	return self;
}

void nQtreeDestroy(sQtree* self) {
	if (self) {
		if (self->children) nArrayDestroy(self->children);
		if (self->elements) nListDestroy(self->elements);
		if (!self->parent) {
			free(self->counter);
		}
		free(self);
	}
}

//acessors and mutators
sRect nQtreePosition(sQtree* self) {
	return self->pos;
}

//methods
void nQtreeInsert(sQtree* self, sQtreeElem* element) {

	const sRect* elemPos = element->posGetter(element->elem);
		
	if (hasIntersection(&self->pos, elemPos)) {

		if (self->children) {
			for (Uint32 i = 0; i < nArraySize(self->children); i++) {
				sQtree* child = nArrayAt(self->children, i);
				nQtreeInsert(child, element);
			}
		}
		else if (!self->elements) {			
			self->elements = nListCreate();
			nListPush(self->elements, element, NULL);				
		}
		else if (nListSize(self->elements) < MAX_ELEMENTS ||			
			(self->pos.w / 2) < MIN_LENGTH) {
			if(!nListContains(self->elements, element)) nListPush(self->elements, element, NULL);
		}
		else if (nListSize(self->elements) >= MAX_ELEMENTS) {			
			//first subdivide
			subdivide(self);
			//then, insert element recursively
			nQtreeInsert(self, element);
		}
	}
}

void nQtreeRemove(sQtree* self, sQtreeElem* element) {	
	const sRect* elemPos = element->posGetter(element->elem);
	if ((!self->elements && !self->children) || 
		!hasIntersection(&self->pos, elemPos)) return;
	if (self->children) {
		for (Uint32 i = 0; i < nArraySize(self->children); i++) {
			sQtree* child = nArrayAt(self->children, i);
			nQtreeRemove(child, element);			
		}
	}
	else nListRemove(self->elements, element);
}

void nQtreeUpdate(sQtree* self, sQtreeElem* element, sRect previous) {	
	
	const sRect* elemPos = element->posGetter(element->elem);	
	
	bool has = hasIntersection(&self->pos, elemPos);
	bool had = hasIntersection(&self->pos, &previous);

	if (self->children && (had || has)) {
		for (Uint32 i = 0; i < nArraySize(self->children); i++) {
			sQtree* child = nArrayAt(self->children, i);
			nQtreeUpdate(child, element, previous);
		}
	}
	else if (had && !has) {
		if (self->elements) nListRemove(self->elements, element);
	}
	else if (!had && has) {
		nQtreeInsert(self, element);
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
	self->children = nArrayCreate();
	nArrayPush(self->children, nQtreeCreate(self, qtree01), nQtreeDestroy);
	nArrayPush(self->children, nQtreeCreate(self, qtree02), nQtreeDestroy);
	nArrayPush(self->children, nQtreeCreate(self, qtree03), nQtreeDestroy);
	nArrayPush(self->children, nQtreeCreate(self, qtree04), nQtreeDestroy);

	//then transfer all entities to childrens
	for (sQtreeElem* elem = nListBegin(self->elements); elem != NULL; 
		elem = nListNext(self->elements)) 
	{
		nQtreeInsert(self, elem);
	}

	//finally, delete and nullify self->elements
	nListDestroy(self->elements);
	self->elements = NULL;
}


void nQtreeGetElementsInArea(sQtree* self, const sRect* area, sArray* arr, bool begin){
	
	if (begin) {
		*self->counter = *self->counter + 1; 
	}

	if (hasIntersection(&self->pos, area)) {

		for (sQtreeElem* elem = nListBegin(self->elements); elem != NULL; 
				elem = nListNext(self->elements)){			

			if (elem->flag != *self->counter) {
				elem->flag = *self->counter;
				const sRect* elemPos = elem->posGetter(elem->elem);	
				if(hasIntersection(elemPos, area)){									
					nArrayPush(arr, elem, NULL);				
				}
			}			
		}

		if (self->children) {
			for (Uint32 i = 0; i < nArraySize(self->children); i++) {
				sQtree* child = nArrayAt(self->children, i);
				nQtreeGetElementsInArea(child, area, arr, false);
			}
		}
	}
}

