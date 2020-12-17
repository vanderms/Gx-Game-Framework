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
        for (Uint32 i = 0; i < nArraySize(self->sequence); i++) {
            int index = *(int*) nArrayAt(self->sequence, i);
            sImage* image = NULL;
            if(index != -1){
                const char* imageName = sf("%s|%d", self->group, index);
                image = nFolderGetImage(self->folder, imageName);
                nUtilAssertResourceNotFound(image);
            }
            nArrayInsert(self->images, i,  image, NULL);
        }
    }
    else {
        sImage* image = nFolderGetImage(self->folder, self->group);
        nUtilAssertResourceNotFound(image);
        for (Uint32 i = 0; i < (Uint32) self->matrix.nr * self->matrix.nc; i++) {
            nArrayInsert(self->images, i, image, NULL);
        }
    }
}

static void onRender(sEvent* e) {

    sTilemap* self = e->target;
    const sRect* elemPos = nElemPosition(self->base);
    sRect target = nElemCalcPosOnCamera(self->base);
    int opacity = nElemOpacity(self->base);

    int w = (elemPos->w / self->matrix.nc);
    int h = (elemPos->h / self->matrix.nr);

    int rowStart = 0;
    int rowEnd = self->matrix.nr;
    int columnStart = 0;
    int columnEnd =  self->matrix.nc;

    sSize windowSize = nAppLogicalSize();

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
            sImage* image = nArrayAt(self->images, index);
            if(!image){
                continue;
            }
            int x = (target.x + columns * w);
            const sSize* imageSize = nImageSize(image);
            //calc child pos
            sRect pos = {
                .x  = x - ((imageSize->w - w) / 2), //...xcenter texture
                .y = y - ((imageSize->h - h) / 2), //... ycenter texture
                .w = imageSize->w,
                .h = imageSize->h
            };
            nImageRender(image, &pos, 0.0, SDL_FLIP_NONE, opacity);
        }
    }
}

static void onDestroy(sEvent* e) {
    sTilemap* self = e->target;
    nArrayDestroy(self->images);
    nArrayDestroy(self->sequence);
    free(self->group);
    free(self);
}

void nTilemapCreate(sElement* base,
    const char* path, sMatrix matrix, const int* sequence)
{

	nUtilAssertArgument(base && path);
	nUtilAssertArgument(nElemPosition(base) && nElemIsRenderable(base));
	nUtilAssertArgument(matrix.nr && matrix.nc);

	sTilemap* self =nUtilAssertAlloc(calloc(1, sizeof(sTilemap)));
	self->base = base;
	self->matrix = matrix;

    sArray* tokens = nAppTokenize(path, "/");
   nUtilAssertArgument(nArraySize(tokens) == 2);
    self->folder = nAppGetFolder(nArrayAt(tokens, 0));
   nUtilAssertArgument(self->folder);
    self->group = nUtilCreateString(nArrayAt(tokens, 1));

    self->images = nArrayCreate();

    if (sequence) {
         self->sequence = nArrayCreate();
         for (Uint32 i = 0; i < (Uint32) matrix.nr * matrix.nc; i++) {
            nArrayPush(self->sequence, nUtilCreateInt(sequence[i]), free);
        }
    }
    else self->sequence = NULL;
    updateImages(self);

    nElemAddComponent(self->base, &(sComponent){
        .name = "nTilemap",
        .target = self,
        .onDestroy = onDestroy,
        .onRender = onRender,
    });
}

