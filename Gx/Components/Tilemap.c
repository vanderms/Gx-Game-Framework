#include "Components.h"
#include <string.h>

typedef struct Tilemap {	
	Element* base;	
	Matrix matrix;	
	char* path;
	char* folder;
	char* group;	
}Tilemap;

static void onDestroy(GxEvent* e) {
	Tilemap* self = e->target;
	folder->removeAsset(self->path);
	free(self->folder);
	free(self->group);
	free(self->path);
	free(self);
}

static void setTilemapImage(Tilemap* self, const int* sequence) {

}

static void updateTilemap(Element* self, const int* sequence) {
	Tilemap* self = elem->getTarget(self);

}

static GxElement* createTileMap(const GxIni* ini, const char* path, Matrix matrix, const int* sequence) {
	
	util->assertArgument(ini->position && ini->position->w && ini->position->h &&
		matrix.nr && matrix.nc && path);
	util->assertArgument(ini->display == elem->ABSOLUTE || ini->display == elem->RELATIVE);
	
	Tilemap* self = util->assertAlloc(calloc(1, sizeof(Tilemap)));	
	self->matrix = matrix;
	
	char folderName[32], group[32];
	util->splitAssetPath(path, folderName, group);

	
	folder->createTilemap(folderName, 
		group, (GxSize) { ini->position->w, ini->position->h }, ini->matrix, ini->sequence
	);
			
	self->base = GxCreateElement(ini);
	GxTilemapSetImage_(self->base, self->pallete);
	
	self->folder = GmCreateString(folder);
	self->group = GmCreateString(group);
	
	GxElemSetChild(self->base, self);
	GxScene* scene = GxElemGetScene(self->base);

	//add event listeners
	GxSceneAddEventListener(scene, GxEventOnDestroy, onDestroyTilemap, self);	
		
	return self->base;
}


