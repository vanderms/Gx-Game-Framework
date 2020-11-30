#include "../Graphics/GxGraphics.h"
#include "../Scene/GxScene.h"
#include "../Array/Array.h"
#include "../Quadtree/GxQuadtree.h"
#include "../Element/GxElement.h"
#include "../Private/GxElement.h"
#include "../Private/GxGraphicAssets.h"
#include "../Renderable/GxRenderable.h"
#include "../Folder/GxFolder.h"
#include "../Folder/GxFolder.h"
#include "../List/GxList.h"
#include "../Map/GxMap.h"

typedef struct GxGraphics {
	GxScene* scene;
	GxQtree* rtree;
	sArray* absolute;
	sArray* renderables;	
}GxGraphics;

GxGraphics* GxCreateGraphics_(GxScene* scene){
	GxGraphics* self = malloc(sizeof(GxGraphics));
	nsUtil->assertAlloc(self);
	self->scene = scene;
	GxSize size = GxSceneGetSize(scene);
	int length = size.w > size.h ? size.w : size.h ;	
	self->rtree = GxCreateQtree_(NULL, (SDL_Rect) { 0, 0, length, length }, "graphical");	
	self->absolute = nsArr->create();
	self->renderables = nsArr->create();
	return self;
}

void GxDestroyGraphics_(GxGraphics* self) {
	if (self) {
		void GxDestroyQtree_(GxQtree* self);
		GxDestroyQtree_(self->rtree);
		nsArr->destroy(self->absolute);
		nsArr->destroy(self->renderables);	
		free(self);
	}
}

void GxGraphicsInsertElement_(GxGraphics* self, GxElement* element) {	
	if (GxElemIsRenderable(element)) {
		if(GxElemHasRelativePosition(element)) GxQtreeInsert_(self->rtree, element);
		else if(GxElemHasAbsolutePosition(element)) nsArr->push(self->absolute, element, NULL);
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
		else if(GxElemHasAbsolutePosition(element)) nsArr->removeByValue(self->absolute, element);
	}
}

static inline void fillRenderables_(GxElement* element) {
	GxGraphics* graphics = GxSceneGetGraphics(GxElemGetScene(element));
	if (!GxElemIsHidden(element)) {
		nsArr->push(graphics->renderables, element, NULL);
	}
}

static inline int compareIndexes_(GxElement* lhs, GxElement* rhs) {
	int i01 = GxElemGetZIndex(lhs);
	int i02 = GxElemGetZIndex(rhs);
	return i01 == i02 ? 0 : (i01 > i02 ? 1 : -1);
}

void GxGraphicsUpdate_(GxGraphics* self) {	
		
	nsArr->reserve(self->renderables, 200);

	//fill renderables with absolute elements
	for (Uint32 i = 0; i < nsArr->size(self->absolute); i++){	
		GxElement* e = nsArr->at(self->absolute, i);
		if(GxElemIsHidden(e)) continue;
		GxSize size = GxSceneGetSize(self->scene);
		SDL_Rect pos = (SDL_Rect){ 0, 0, size.w, size.h };
		const SDL_Rect* elemPos = GxElemGetPosition(e);
		if (SDL_HasIntersection(&pos, elemPos)) {
			nsArr->push(self->renderables, e, NULL);
		}
	}

	//fill with relative elements
	const SDL_Rect* area = GxElemGetPosition(GxSceneGetCamera(self->scene));	
	GxQtreeIterate_(self->rtree, *area, fillRenderables_, true);

	//sort
	nsArr->sort(self->renderables, (GxComp) compareIndexes_);

	//then iterate
	for (Uint32 i = 0; i < nsArr->size(self->renderables); i++){
		GxElement* elem = nsArr->at(self->renderables, i);		
		GxElemRender_(elem);
	}
	nsArr->clean(self->renderables);
}


//...ELEMENT RENDER METHODS
static SDL_Rect calcAbsolutePos(GxElement* self) {
	int y = nsApp->logicalSize().h - (self->pos->y + self->pos->h);
	return (SDL_Rect) { self->pos->x, y, self->pos->w, self->pos->h };
}

static SDL_Rect calcRelativePos(GxElement* self) {
	const SDL_Rect* cpos = GxSceneGetCamera(self->scene)->pos;
	int x = self->pos->x - cpos->x;
	int y = (cpos->y + cpos->h) - (self->pos->y + self->pos->h);
	return (SDL_Rect) { x, y, self->pos->w, self->pos->h };
}

static void applyWidgetData(GxElement* self, SDL_Rect* pos, GxImage* image) {

	GxSize imgsize = GxImageGetSize_(image);
	pos->w = (int) (imgsize.w * self->renderable->proportion + 0.5);
	pos->h = (int) (imgsize.h * self->renderable->proportion + 0.5);

	//horizontal alignment -> (left is default)
	if(!self->renderable->alignment ||
		self->renderable->alignment->horizontal == sCenter) {
		pos->x += (self->pos->w - pos->w) / 2;
	}
	else if (self->renderable->alignment->horizontal ==  sRight){
		pos->x += self->pos->w - pos->w;
	}
	else if(self->renderable->alignment->horizontal == sNum) {
		pos->x += (self->pos->w - pos->w) / 2; //first set default
		pos->x += self->renderable->alignment->x;
	}

	//vertical alignment (top is default)
	if (!self->renderable->alignment ||
		self->renderable->alignment->vertical == sCenter){
		pos->y += (self->pos->h - pos->h) / 2;
	}
	else if (self->renderable->alignment->vertical == sBottom) {
		pos->y += (self->pos->h - pos->h);
	}
	else if(self->renderable->alignment->vertical == sNum) {
		pos->y += (self->pos->h - pos->h) / 2; //first set default
		pos->y -= self->renderable->alignment->y;
	}
}

static inline SDL_Rect* calcAbsoluteImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	applyWidgetData(self, pos, image);
	return pos;
}

static inline SDL_Rect* calcRelativeImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	applyWidgetData(self, pos, image);
	return pos;
}

 SDL_Rect* GxElemCalcImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	return (
		self->renderable->type == GxElemAbsolute ?
		calcAbsoluteImagePos(self, pos, image) :
		calcRelativeImagePos(self, pos, image)
	);
}

SDL_Rect GxGetElemPositionOnWindow(GxElement* self) {
	validateElem(self, false, true);
	return (
		self->renderable->type == GxElemAbsolute ?
		calcAbsolutePos(self) :
		calcRelativePos(self)
	);
}

static void renderBorder(SDL_Renderer* renderer, SDL_Rect* pos, int quantity) {
	if (quantity <= 0) return;
	if (pos->w == 0 || pos->h == 0) return;	
	SDL_RenderDrawRect(renderer, pos);
	pos->x++;
	pos->y++;
	pos->w -= 2;
	pos->h -= 2;
	renderBorder(renderer, pos, quantity - 1);
}

void GxElemRender_(GxElement* self) {

	SDL_Renderer* renderer = nsApp->SDLRenderer();
	SDL_Rect pos = GxGetElemPositionOnWindow(self);
	SDL_Rect labelPos = pos;
	SDL_Color* color = self->renderable->backgroundColor->value;

	//first backgrund color
	if (color && color->a != 0) {
		int bs = self->renderable->border.size;
		SDL_Rect square = {pos.x + bs, pos.y + bs, pos.w - 2*bs, pos.h - 2*bs};
		SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
		SDL_Rect* dst = nsApp->calcDest(&square, &(SDL_Rect){0});
		SDL_RenderFillRect(renderer, dst);
		//SDL_RenderDrawRect(renderer, &pos);
	}

	//then borders
	SDL_Color* borderColor = self->renderable->border.color->value;
	int bsize = self->renderable->border.size;

	if (bsize > 0 && borderColor && borderColor->a) {
		SDL_SetRenderDrawColor(renderer,
			borderColor->r, borderColor->g, borderColor->b, borderColor->a
		);
		SDL_Rect* dst = nsApp->calcDest(&pos, &(SDL_Rect){0});
		renderBorder(renderer, dst, bsize);
	}

	//then image or animation
	GxImage* image = NULL;
	if (self->renderable->image) {
		image = self->renderable->image;
	}
	else if (self->renderable->animation) {
		GxAnimation* anim = self->renderable->animation; //create alias
		self->renderable->animCounter++; //to avoid counter starting with 0.
		if (self->renderable->animCurrent >= GxAnimGetQuantity_(anim)) {
			self->renderable->animCurrent = 0;
		}
		image = GxAnimGetImage_(anim, self->renderable->animCurrent);

		if (self->renderable->animCounter % GxAnimGetInterval_(anim) == 0)
			self->renderable->animCurrent++;

			//if it's the last frame in animation and there's no repeat set renderable->animation to NULL
		if (self->renderable->animCurrent >= GxAnimGetQuantity_(anim) && !GxAnimIsContinous_(anim)) {
			self->renderable->animation = NULL;
			self->renderable->animCounter = 0;
			self->renderable->animCurrent = 0;
		}
	}

	if (image) {
		GxElemCalcImagePos(self, &pos, image);
		GxImageRender_(image, &pos, self->renderable->angle, 
			self->renderable->orientation, self->renderable->opacity
		);
	}

	//finally, render label
	if (self->renderable->shouldUpdateLabel) {
		GxElementUpdateLabel_(self->renderable);
	}
	if (self->renderable->label) {
		GxElemCalcImagePos(self, &labelPos, self->renderable->label);
		GxImageRender_(self->renderable->label, &labelPos, 0.0, 
			(SDL_RendererFlip) GxElemForward, self->renderable->opacity
		);
	}
}

//ASSETS RENDER METHODS
void GxImageRender_(GxImage* self, SDL_Rect* target, double angle, SDL_RendererFlip orientation, Uint8 opacity) {

    if (self->type == Texture || self->type == Opaque || self->type == Text){
        void* resource = NULL;
        void* src = NULL;
        if (self->type == Texture || self->type == Text) {
            resource = self->resource;
            src = self->src;
        }
        else { // === Opaque
            resource =  self->source->resource;
            src = self->src;
        }
	    if (resource && opacity) {
            if (opacity != 255) {
                 SDL_SetTextureAlphaMod(resource, opacity);
            }           
            SDL_Renderer* renderer = nsApp->SDLRenderer();
            SDL_Rect* dst = ( self->type == Text ? 
                nsApp->calcLabelDest(target, &(SDL_Rect){0}) 
                : nsApp->calcDest(target, &(SDL_Rect){0, 0, 0, 0})
            );
             
            if ((angle <= -1.0 || angle >= 1.0) || orientation != SDL_FLIP_NONE) {
                SDL_RenderCopyEx(renderer, resource, src, dst, angle, NULL, orientation);
            }
            else {
                SDL_RenderCopy(renderer, resource, src, dst);
            }
            if(opacity != 255){
                SDL_SetTextureAlphaMod(resource, 255);
            }
        }
	}
	else if (self->type == Palette) {
		GxImageRenderTilePalette_(self, target, opacity);
	}
}

void GxImageRenderTilePalette_(GxImage* self, SDL_Rect* target, Uint8 opacity) {
	
    if (!GxFolderHasStatus_(self->folder, GxStatusReady)){ return; }

    int w = (self->size.w / self->matrix.nc);
    int h = (self->size.h / self->matrix.nr);

    int rowStart = 0;
    int rowEnd = self->matrix.nr;
    int columnStart = 0;
    int columnEnd =  self->matrix.nc;

    GxSize windowSize = nsApp->logicalSize();

    //... calc renderable area of the matrix
    if (target->x < 0) {
        columnStart = -target->x / w;
    }
    if (target->x + target->w > windowSize.w) {
        columnEnd -= (target->x + target->w - windowSize.w) / w;
    }
    if (target->y < 0) {
        rowStart = -target->y / h;
    }
    if (target->y + target->h > windowSize.h) {
        rowEnd -= (target->y + target->h - windowSize.h) / h;
    }

    //...
    for (int rows = rowStart; rows < rowEnd; rows++) {

       int y = target->y + rows * h;

        for (int columns = columnStart; columns < columnEnd; columns++) {

            int index = rows * self->matrix.nc + columns;
            GxImage* child = nsArr->at(self->children, index);
            if(child->type == Blank){ continue; }
            int x = (target->x + columns * w);

            //calc child pos
            SDL_Rect pos = {
                .x  = x - ((child->size.w - w) / 2), //...xcenter texture
                .y = y - ((child->size.h - h) / 2), //... ycenter texture
                .w = child->size.w,
                .h = child->size.h
            };           
            GxImageRender_(child, &pos, 0.0, SDL_FLIP_NONE, opacity);
        }
    }
}