#include "Tilemap.h"

typedef struct sTilemap {	
	sElement* base;
    sFolder* folder;
	char* group;
	sMatrix matrix;
	sArray* sequence;
    sArray* images;
} sTilemap;


/*
 I am not sure if or how to implement the update. The risk of accessing the int sequence out of range (which is
 already big on creation) is much bigger on updating. And for now I cannot think in a safe and practical way to do it.
*/

//...auxiliary
static void updateImages(sTilemap* self){
    
    if (self->sequence) {
        for (Uint32 i = 0; i < nArray->size(self->sequence); i++) {
            int index = *(int*) nArray->at(self->sequence, i);
            sImage* image = NULL;
            if(index != -1){
                const char* imageName = nApp->sf("%s|%d", self->group, index);
                image = nFolder->getImage(self->folder, imageName); 
                nUtil->assertResourceNotFound(image);
            }
            nArray->insert(self->images, i,  image, NULL);
        }
    }
    else {
        sImage* image = nFolder->getImage(self->folder, self->group);
        nUtil->assertResourceNotFound(image);
        for (Uint32 i = 0; i < (Uint32) self->matrix.nr * self->matrix.nc; i++) {
            nArray->insert(self->images, i, image, NULL);
        }
    }
}

static void onRender(sEvent* e) {

    sTilemap* self = e->target;
    const sRect* elemPos = nElem->position(self->base);
    sRect target = nElem->style->calcPosOnCamera(self->base);
    int opacity = nElem->style->opacity(self->base);
   
    int w = (elemPos->w / self->matrix.nc);
    int h = (elemPos->h / self->matrix.nr);

    int rowStart = 0;
    int rowEnd = self->matrix.nr;
    int columnStart = 0;
    int columnEnd =  self->matrix.nc;

    sSize windowSize = nApp->logicalSize();

    //... calc renderable area of the matrix
    if (target.x < 0) {
        columnStart = -target.x / w;
    }
    if (target.x + target.w > windowSize.w) {
        columnEnd -= (target.x + target.w - windowSize.w) / w;
    }
    if (target.y < 0) {
        rowStart = -target.y / h;
    }
    if (target.y + target.h > windowSize.h) {
        rowEnd -= (target.y + target.h - windowSize.h) / h;
    }

    //...
    for (int rows = rowStart; rows < rowEnd; rows++) {

       int y = target.y + rows * h;

        for (int columns = columnStart; columns < columnEnd; columns++) {

            int index = rows * self->matrix.nc + columns;
            sImage* image = nArray->at(self->images, index);
            if(!image){ 
                continue; 
            }
            int x = (target.x + columns * w);
            const sSize* imageSize = nFolder->img->size(image);
            //calc child pos
            sRect pos = {
                .x  = x - ((imageSize->w - w) / 2), //...xcenter texture
                .y = y - ((imageSize->h - h) / 2), //... ycenter texture
                .w = imageSize->w,
                .h = imageSize->h
            };           
            nFolder->img->render(image, &pos, 0.0, SDL_FLIP_NONE, opacity);
        }
    }
}

static void onDestroy(sEvent* e) {
    sTilemap* self = e->target;
    nArray->destroy(self->images);
    nArray->destroy(self->sequence);
    free(self->group);
    free(self);
}

static void implement(sElement* base, const char* path, sMatrix matrix, const int* sequence) {
	
	nUtil->assertArgument(base && path);
	nUtil->assertArgument(nElem->position(base) && nElem->isRenderable(base));
	nUtil->assertArgument(matrix.nr && matrix.nc);
			
	sTilemap* self = nUtil->assertAlloc(calloc(1, sizeof(sTilemap)));	
	self->base = base;
	self->matrix = matrix;
    
    sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    self->folder = nApp->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(self->folder);
    self->group = nUtil->createString(nArray->at(tokens, 1));    
   
    self->images = nArray->create();
	
    if (sequence) {
         self->sequence = nArray->create();
         for (Uint32 i = 0; i < (Uint32) matrix.nr * matrix.nc; i++) {
            nArray->push(self->sequence, nUtil->createInt(sequence[i]), free);
        }
    }
    else self->sequence = NULL;
    updateImages(self);	

    nElem->addComponent(self->base, &(sComponent){
        .name = "nTilemap",
        .target = self,        
        .onDestroy = onDestroy,
        .onRender = onRender,
    });
}

const struct sTilemapNamespace* const nTilemap = &(struct sTilemapNamespace) {
	.implement = implement
};