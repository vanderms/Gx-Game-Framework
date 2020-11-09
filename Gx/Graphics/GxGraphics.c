#include "../Graphics/GxGraphics.h"
#include "../Scene/GxScene.h"
#include "../Array/GxArray.h"
#include "../Quadtree/GxQuadtree.h"
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../Folder/GxFolder.h"
#include "../List/GxList.h"
#include "../Map/GxMap.h"

typedef struct GxGraphics {
	GxScene* scene;
	GxQtree* rtree;
	GxArray* absolute;
	GxArray* renderables;	
}GxGraphics;

GxGraphics* GxCreateGraphics_(GxScene* scene){
	GxGraphics* self = malloc(sizeof(GxGraphics));
	GxAssertAllocationFailure(self);
	self->scene = scene;
	GxSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w : size.h ;	
	self->rtree = GxCreateQtree_(NULL, (SDL_Rect) { 0, 0, length, length }, "graphical");	
	self->absolute = GxCreateArray();
	self->renderables = GxCreateArray();
	return self;
}

void GxDestroyGraphics_(GxGraphics* self) {
	if (self) {
		void GxDestroyQtree_(GxQtree* self);
		GxDestroyQtree_(self->rtree);
		GxDestroyArray(self->absolute);
		GxDestroyArray(self->renderables);	
		free(self);
	}
}

void GxGraphicsInsertElement_(GxGraphics* self, GxElement* element) {	
	if (GxElemIsRenderable(element)) {
		if(GxElemHasRelativePosition(element)) GxQtreeInsert_(self->rtree, element);
		else if(GxElemHasAbsolutePosition(element)) GxArrayPush(self->absolute, element, NULL);
	}
}

void GxGraphicsUpdatePosition_(GxGraphics* self, GxElement* element, SDL_Rect previousPos) {	
	if (GxElemHasRelativePosition(element)) {
		GxQtreeUpdate_(self->rtree, element, previousPos);
	}
}

void GxGraphicsRemoveElement_(GxGraphics* self, GxElement* element) {	
	if (GxElemIsRenderable(element)) {
		if(GxElemHasRelativePosition(element)) GxQtreeRemove_(self->rtree, element);
		else if(GxElemHasAbsolutePosition(element)) GxArrayRemoveByValue(self->absolute, element);
	}
}

static inline void fillRenderables_(GxElement* element) {
	GxGraphics* graphics = GxSceneGetGraphics(GxElemGetScene(element));
	if (!GxElemIsHidden(element)) {
		GxArrayPush(graphics->renderables, element, NULL);
	}
}

static inline int compareIndexes_(GxElement* lhs, GxElement* rhs) {
	int i01 = GxElemGetZIndex(lhs);
	int i02 = GxElemGetZIndex(rhs);
	return i01 == i02 ? 0 : (i01 > i02 ? 1 : -1);
}

void GxGraphicsUpdate_(GxGraphics* self) {	
		
	GxArrayReserve(self->renderables, 200);

	//fill renderables with absolute elements
	for (Uint32 i = 0; i < GxArraySize(self->absolute); i++){	
		GxElement* e = GxArrayAt(self->absolute, i);
		if(GxElemIsHidden(e)) continue;
		GxSize size = GxSceneGetSize(self->scene);
		SDL_Rect pos = (SDL_Rect){ 0, 0, size.w, size.h };
		const SDL_Rect* elemPos = GxElemGetPosition(e);
		if (SDL_HasIntersection(&pos, elemPos)) {
			GxArrayPush(self->renderables, e, NULL);
		}
	}

	//fill with relative elements
	const SDL_Rect* area = GxElemGetPosition(GxSceneGetCamera(self->scene));	
	GxQtreeIterate_(self->rtree, *area, fillRenderables_, true);

	//sort
	GxArraySort(self->renderables, (GxComp) compareIndexes_);

	//then iterate
	for (Uint32 i = 0; i < GxArraySize(self->renderables); i++){
		GxElement* elem = GxArrayAt(self->renderables, i);		
		GxElemRender_(elem);
	}
	GxArrayClean(self->renderables);
}
