#include "../Graphics/Graphics.h"
#include "../Scene/Scene.h"
#include "../Array/Array.h"
#include "../Qtree/Qtree.h"
#include "../Element/Element.h"
#include "../Folder/Folder.h"
#include "../List/List.h"
#include "../Map/Map.h"

typedef struct sGraphics {
	sScene* scene;
	sQtree* rtree;
	sArray* absolute;
	sArray* renderables;	
}sGraphics;

static sGraphics* create(sScene* scene){
	sGraphics* self = malloc(sizeof(sGraphics));
	nUtil->assertAlloc(self);
	self->scene = scene;
	sSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w : size.h ;	
	self->rtree =nQtree->create(NULL, (sRect) { 0, 0, length, length }, nQtree->GRAPHICAL);	
	self->absolute = nArray->create();
	self->renderables = nArray->create();
	return self;
}

static void destroy(sGraphics* self) {
	if (self) {		
		nQtree->destroy(self->rtree);
		nArray->destroy(self->absolute);
		nArray->destroy(self->renderables);	
		free(self);
	}
}

static void insert(sGraphics* self, sElement* element) {	
	if (nElem->isRenderable(element)) {
		if(nElem->style->hasRelativePosition(element)){
			nQtree->insert(self->rtree, element);
		}
		else if(nElem->style->hasAbsolutePosition(element)){
			nArray->push(self->absolute, element, NULL);
		}
	}
}

static void updateElement(sGraphics* self, sElement* element, sRect previousPos) {	
	if (nElem->style->hasRelativePosition(element)) {
		nQtree->update(self->rtree, element, previousPos);
	}
}

static void removeElement(sGraphics* self, sElement* element) {	
	if (nElem->isRenderable(element)) {
		if(nElem->style->hasRelativePosition(element)) nQtree->remove(self->rtree, element);
		else if(nElem->style->hasAbsolutePosition(element)) nArray->removeByValue(self->absolute, element);
	}
}

static void iFillRenderables(sElement* elem) {
	sGraphics* self = GxSceneGetGraphics(nElem->scene(elem));
	if (!nElem->style->isHidden(elem)) {
			nArray->push(self->renderables, elem, NULL);
	}	
}

static int compareIndexes_(sElement* lhs, sElement* rhs) {
	int i01 = nElem->style->zIndex(lhs);
	int i02 = nElem->style->zIndex(rhs);
	return i01 == i02 ? 0 : (i01 > i02 ? 1 : -1);
}

static void update(sGraphics* self) {	
		
	nArray->reserve(self->renderables, 200);

	//fill renderables with absolute elements
	for (Uint32 i = 0; i < nArray->size(self->absolute); i++){	
		sElement* e = nArray->at(self->absolute, i);
		if(nElem->style->isHidden(e)) continue;
		sSize size = GxSceneGetSize(self->scene);
		sRect pos = (sRect){ 0, 0, size.w, size.h };
		const sRect* elemPos = nElem->position(e);
		if (SDL_HasIntersection(&pos, elemPos)) {
			nArray->push(self->renderables, e, NULL);
		}
	}

	//fill with relative elements
	const sRect* area = nElem->position(GxSceneGetCamera(self->scene));
	sArray* temp = nArray->create();
	nQtree->getAllElementsInArea(self->rtree, *area, temp, true);
	for (Uint32 i = 0; i < nArray->size(temp); i++) {
		iFillRenderables(nArray->at(temp, i));
	}
	nArray->destroy(temp);	

	//sort
	nArray->sort(self->renderables, (GxComp) compareIndexes_);

	//then iterate
	for (Uint32 i = 0; i < nArray->size(self->renderables); i++){
		sElement* elem = nArray->at(self->renderables, i);		
		nGraphics->renderElement(elem);
	}
	nArray->clean(self->renderables);
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

static void renderElement(sElement* self) {	

	SDL_Renderer* renderer = nApp->SDLRenderer();	
	sRect pos = nElem->style->calcPosOnCamera(self);
	sRect labelPos = pos;
	const SDL_Color* color = nElem->style->backgroundColor(self);

	//...first call element onRender
	nElem->style->p->onRender(self);

	//...then render background color
	if (color && color->a != 0) {
		int bs = nElem->style->borderSize(self);
		sRect square = {pos.x + bs, pos.y + bs, pos.w - 2*bs, pos.h - 2*bs};
		SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
		sRect* dst = nApp->calcDest(&square, &(sRect){0});
		SDL_RenderFillRect(renderer, dst);		
	}

	//... borders
	const SDL_Color* borderColor = nElem->style->borderColor(self);
	int bsize = nElem->style->borderSize(self);;

	if (bsize > 0 && borderColor && borderColor->a) {
		SDL_SetRenderDrawColor(renderer,
			borderColor->r, borderColor->g, borderColor->b, borderColor->a
		);
		sRect* dst = nApp->calcDest(&pos, &(sRect){0});
		renderBorder(renderer, dst, bsize);
	}

	//... image or animation
	sImage* image = NULL;

	if ((image =  nElem->style->p->getImageRef(self))) {
		
		nElem->style->p->calcImagePosOnCamera(self, &pos, image);
		
		nFolder->p->renderImage(image, &pos, nElem->style->angle(self), 
			(SDL_RendererFlip) nElem->style->orientation(self), 
			nElem->style->opacity(self)
		);
	}

	//...finally, render label
	if ((image = nElem->style->p->label(self))) {
		
		nElem->style->p->calcImagePosOnCamera(self, &labelPos, image);
		
		nFolder->p->renderImage(image, &labelPos, 0.0, 
			(SDL_RendererFlip)  nElem->orientation->FORWARD, 
			nElem->style->opacity(self)
		);
	}
}

const struct sGraphicsNamespace* nGraphics = &(struct sGraphicsNamespace){
	.create = create,
	.destroy = destroy,
	.insert = insert,
	.updateElement = updateElement,
	.remove = removeElement,
	.update = update,	
	.renderElement = renderElement
};