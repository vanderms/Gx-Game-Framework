#include "../Tilemap/GxTilemap.h"
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../Folder/GxFolder.h"
#include "../Scene/GxScene.h"
#include <string.h>

typedef struct Tilemap {
	Uint32 hash;
	GxElement* base;
	Uint32 size;
	GxMatrix matrix;
	int* sequence;
	char* folder;
	char* group;
	GxImage* pallete;
}Tilemap;

static const Uint32 tilemapHash = 1118096401;


bool GxIsTilemap(GxElement* elem) {
	Tilemap* self = GxElemGetChild(elem);
	return self && (self->hash == tilemapHash);
}

static void onDestroyTilemap(GxEvent* e) {
	Tilemap* self = e->target;
	free(self->folder);
	free(self->group);
	free(self->sequence);
	self->hash = 0;
	GxDestroyImage_(self->pallete);
	free(self);
}

GxElement* GxCreateTileMap(const char* tilePath, const GxIni* ini) {
	
	GxAssertInvalidArgument(ini->position && ini->position->w && ini->position->h &&
		ini->matrix.nr && ini->matrix.nc && tilePath);
	GxAssertInvalidArgument(ini->display == GxElemAbsolute || ini->display == GxElemRelative);
	
	Tilemap* self = calloc(1, sizeof(Tilemap));
	GxAssertAllocationFailure(self);
	self->hash = tilemapHash;
	self->matrix = ini->matrix;
	self->size = (Uint32) (ini->matrix.nc * ini->matrix.nr);
	if (ini->sequence) {		
		self->sequence = malloc(self->size * sizeof(int));
		GxAssertAllocationFailure(self->sequence);
		memcpy(self->sequence, ini->sequence, self->size * sizeof(int));	
	}
	
	char folder[32], group[32];
	GxSplitAssetPath_(tilePath, folder, group);
	
	self->pallete = GxCreateTilePalette_(GxGetFolder_(folder), 
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


