#include "../Graphics/Graphics.h"
#include "../Scene/Scene.h"
#include "../Containers/Array/Array.h"
#include "../Containers/Qtree/Qtree.h"
#include "../Element/Element.h"
#include "../Folder/Folder.h"
#include "../Containers/List/List.h"
#include "../Containers/Map/Map.h"
#include <time.h>

typedef struct sGraphics {
	sScene* scene;
	sQtree* rtree;
	sArray* absolute;
	sArray* renderables;	
}sGraphics;

sGraphics* nGraphicsCreate_(sScene* scene){
	sGraphics* self = malloc(sizeof(sGraphics));
	nUtil->assertAlloc(self);
	self->scene = scene;
	sSize size = nScene->size(scene);
	int length = size.w > size.h ? size.w : size.h ;	
	self->rtree =nQtreeCreate(NULL, (sRect) { 0, 0, length, length });	
	self->absolute = nArrayCreate();
	self->renderables = nArrayCreate();
	return self;
}

void nGraphicsDestroy_(sGraphics* self) {
	if (self) {		
		nQtreeDestroy(self->rtree);
		nArrayDestroy(self->absolute);
		nArrayDestroy(self->renderables);	
		free(self);
	}
}

void nGraphicsInsert_(sGraphics* self, sElement* element) {	
	if (nElemIsRenderable(element)) {
		if(nElemHasRelativeDisplay(element)){
			sQtreeElem* qtreeElem = nElemGetRenderableQtreeElem_(element);
			nQtreeInsert(self->rtree, qtreeElem);
		}
		else if(nElemHasAbsoluteDisplay(element)){
			nArrayPush(self->absolute, element, NULL);
		}
	}
}

void nGraphicsUpdateElement_(sGraphics* self, sElement* element, sRect previousPos) {	
	if (nElemHasRelativeDisplay(element)) {
		sQtreeElem* qtreeElem = nElemGetRenderableQtreeElem_(element);
		nQtreeUpdate(self->rtree, qtreeElem, previousPos);
	}
}

void nGraphicsRemoveElement_(sGraphics* self, sElement* element) {	
	if (nElemIsRenderable(element)) {
		if(nElemHasRelativeDisplay(element)){
			sQtreeElem* qtreeElem = nElemGetRenderableQtreeElem_(element);
			nQtreeRemove(self->rtree, qtreeElem);
		}
		else if(nElemHasAbsoluteDisplay(element)){
			nArrayRemoveByValue(self->absolute, element);
		}
	}
}

static void iFillRenderables(sElement* elem) {
	sGraphics* self = nScene->p_->getGraphics(nElemScene(elem));
	if (!nElemIsHidden(elem)) {
			nArrayPush(self->renderables, elem, NULL);
	}	
}

static int compareIndexes_(sElement* lhs, sElement* rhs) {
	int i01 = nElemZIndex(lhs);
	int i02 = nElemZIndex(rhs);	
	return i01 == i02 ? 0 : (i01 > i02 ? 1 : -1);
}

void nGraphicsUpdate_(sGraphics* self) {	
		
	nArrayReserve(self->renderables, 200);

	//fill renderables with absolute elements
	for (Uint32 i = 0; i < nArraySize(self->absolute); i++){	
		sElement* e = nArrayAt(self->absolute, i);
		if(nElemIsHidden(e)) continue;
		sSize size = nScene->size(self->scene);
		sRect pos = (sRect){ 0, 0, size.w, size.h };
		const sRect* elemPos = nElemPosition(e);
		if (SDL_HasIntersection(&pos, elemPos)) {
			nArrayPush(self->renderables, e, NULL);
		}
	}

	//fill with relative elements
	const sRect* area = nElemPosition(nScene->getCamera(self->scene));
	sArray* temp = nArrayCreate();
	nQtreeGetElementsInArea(self->rtree, area, temp, true);
	for (Uint32 i = 0; i < nArraySize(temp); i++) {		
		iFillRenderables(nQtreeGetElem(nArrayAt(temp, i)));
	}
	nArrayDestroy(temp);	

	//sort	
	nArraySort(self->renderables, (sComp) compareIndexes_);		

	//then iterate
	for (Uint32 i = 0; i < nArraySize(self->renderables); i++){
		sElement* elem = nArrayAt(self->renderables, i);		
		nGraphicsRenderElement_(elem);
	}
	nArrayClean(self->renderables);
}


static void renderBorder(SDL_Renderer* renderer, sRect* pos, int quantity) {
	if (quantity <= 0) return;
	if (pos->w == 0 || pos->h == 0) return;	
	SDL_RenderDrawRect(renderer, pos);
	pos->x++;
	pos->y++;
	pos->w -= 2;
	pos->h -= 2;
	renderBorder(renderer, pos, quantity - 1);
}

void nGraphicsRenderElement_(sElement* self) {	

	SDL_Renderer* renderer = nAppSDLRenderer();	
	sRect pos = nElemCalcPosOnCamera(self);
	sRect labelPos = pos;
	const SDL_Color* color = nElemBackgroundColor(self);

	//...first call element onRender
	nElemOnRender_(self);

	//...then render background color
	if (color && color->a != 0) {
		int bs = nElemBorderSize(self);
		sRect square = {pos.x + bs, pos.y + bs, pos.w - 2*bs, pos.h - 2*bs};
		SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
		sRect* dst = nAppCalcDest(&square, &(sRect){0});
		SDL_RenderFillRect(renderer, dst);		
	}

	//... borders
	const SDL_Color* borderColor = nElemBorderColor(self);
	int bsize = nElemBorderSize(self);;

	if (bsize > 0 && borderColor && borderColor->a) {
		SDL_SetRenderDrawColor(renderer,
			borderColor->r, borderColor->g, borderColor->b, borderColor->a
		);
		sRect* dst = nAppCalcDest(&pos, &(sRect){0});
		renderBorder(renderer, dst, bsize);
	}

	//... image or animation
	sImage* image = NULL;

	if ((image =  nElemGetImageRef_(self))) {
		
		nElemCalcImagePosOnCamera(self, &pos, image);
		
		nImageRender(image, &pos, nElemAngle(self), 
			(SDL_RendererFlip) nElemOrientation(self), 
			nElemOpacity(self)
		);
	}

	//...finally, render label
	if ((image = nElemLabel_(self))) {
		
		nElemCalcImagePosOnCamera(self, &labelPos, image);
		
		nImageRender(image, &labelPos, 0.0, 
			(SDL_RendererFlip)  nElem_FORWARD, 
			nElemOpacity(self)
		);
	}
}

