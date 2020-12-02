#include "../Folder/GxFolder.h"
#include "../Private/GxGraphicAssets.h"
#include "../Map/GxMap.h"
#include "../Utilities/Util.h"
#include "../Scene/GxScene.h"
#include "../sElement/sElement.h"
#include "../App/App.h"
#include <stdbool.h>
#include <string.h>
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <stdio.h>


typedef struct GxFolder {
    char* id;
	int status;

   //folders
	GxMap* assets;

    //load flags
    int assetsLoaded;
    int totalAssets;

    //memory management
    int refCounter;

    //loader
	void(*loader)(void);
} GxFolder;

static GxFolder* sFolder = NULL;

typedef struct GxSound {
    char* id;
    GxFolder* folder;
    Mix_Chunk* chunk;
} GxSound;

typedef struct GxMusic {
    char* id;
    GxFolder* folder;
    Mix_Music* music;
} GxMusic;

//...
void GxCreateFolder(const char* id, void(*loader)(void)) {

    GxFolder* self = malloc(sizeof(GxFolder));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->status = loader ? GxStatusNone : GxStatusReady;
    self->assetsLoaded = 0;
    self->totalAssets = 0;
    self->assets = GmCreateMap();
    self->loader = loader;
    self->refCounter = 0;
    nApp->prv->addFolder(self);
}

void GxDestroyFolder_(GxFolder* self) {

    if (self) {
        GxDestroyMap(self->assets);
        free(self->id);
        free(self);
    }
}

//accessors and mutators
char* GxFolderGetId_(GxFolder* self) {
    return self->id;
}

bool GxFolderHasStatus_(GxFolder* self, int status) {
    return self->status == status;
}

GxImage* GxFolderGetImage_(GxFolder* self, const char* id) {
    return GxMapGet(self->assets, id);
}

Mix_Music* GxFolderGetMusic(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArr->size(tokens) == 2);
    GxFolder* folder = nApp->prv->getFolder(nArr->at(tokens, 0));
    nUtil->assertArgument(folder);
    GxMusic* asset = GxMapGet(folder->assets, nArr->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->music;
}

Mix_Chunk* GxFolderGetChunk(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArr->size(tokens) == 2);
    GxFolder* folder = nApp->prv->getFolder(nArr->at(tokens, 0));
    nUtil->assertArgument(folder);
    GxSound* asset = GxMapGet(folder->assets, nArr->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->chunk;
}

SDL_Texture* GxFolderGetTexture(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArr->size(tokens) == 2);
    GxFolder* folder = nApp->prv->getFolder(nArr->at(tokens, 0));
    nUtil->assertArgument(folder);
    GxImage* asset = GxMapGet(folder->assets, nArr->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->resource;
}

GxAnimation* GxFolderGetAnimation_(GxFolder* self, const char* id) {
     return GxMapGet(self->assets, id);
}

static inline void folderLoad_(GxFolder* self) {
    if(self->status == GxStatusNone){
        self->status = GxStatusLoading;
        sFolder = self;
        self->loader();
        sFolder = NULL;
    }
}

int GXFolderGetPercLoaded_(GxFolder* self) {
    return self->totalAssets ? (self->assetsLoaded * 100 / self->totalAssets) : 100;
}

static inline void folderUnload(GxFolder* self) {
    GxMapClean(self->assets);
    self->status = GxStatusNone;
}

static inline void folderIncreaseAssetsLoaded(GxFolder* self){
    self->assetsLoaded++;
    if(self->assetsLoaded == self->totalAssets){
        self->status = GxStatusReady;
    }
}

void GxFolderIncRefCounter_(GxFolder* self) {
    if (self->refCounter == 0) folderLoad_(self);
    self->refCounter++;
}

void GxFolderDecRefCounter_(GxFolder* self) {
    if(nApp->isRunning()){
        nUtil->assertState(self->refCounter > 0);
        --self->refCounter;
        if (self->refCounter == 0) {
           folderUnload(self);
        }
    }
}

static GxImage* createImage(GxFolder* folder, const char* id, ImageType type) {
    GxImage* self = calloc(1, sizeof(GxImage));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = folder;
    self->type = type;
    GxMapSet(folder->assets, id, self, GxDestroyImage_);
    return self;
}

void GxDestroyImage_(GxImage* self) {
    if (self) {
        if (self->resource){
            SDL_DestroyTexture(self->resource);
        }
        nArr->destroy(self->children);
        free(self->src);
        free(self->id);
        free(self);
    }
}

void GxFolderRemoveAsset(const char* path) {
    char folderName[32];
    char imageName[32];
    nUtil->splitAssetPath(path, folderName, imageName);
    GxFolder* folder = nApp->prv->getFolder(folderName);
    nUtil->assertArgument(folder);
    GxMapRemove(folder->assets, imageName);

}

GxImage* GxImageCreateText_(const char* text, const char* fontName, int size, SDL_Color* color){

    GxImage* self = calloc(1, sizeof(GxImage));
    nUtil->assertAlloc(self);
    self->type = Text;
    const char* fontPath = nApp->prv->getFontPath(fontName);
#if 1
    //for some reason not working
    GxSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nApp->SDLRenderer(), &wsize.w, &wsize.h);
    GxSize lsize = nApp->logicalSize();
    size = ((double) size * wsize.w) / lsize.w;
#endif
    TTF_Font* font = TTF_OpenFont(fontPath, size);
    if(!font){
        nApp->runtimeError(TTF_GetError());
    }
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, *color);
    if (!surface){
        nApp->runtimeError(TTF_GetError());
    }
    self->size.w = surface->w;
    self->size.h = surface->h;
    self->resource = SDL_CreateTextureFromSurface(nApp->SDLRenderer(), surface);
    SDL_FreeSurface(surface);
    return self;
}

void GxLoadImage(const char* id, const char* path, SDL_Rect* src, double proportion) {

    GxFolder* self = sFolder;
    nUtil->assertArgument(self->assets == NULL || GxMapGet(self->assets, id) == NULL);
    nUtil->assertState(self->status == GxStatusLoading);
    GxImage* img = createImage(self, id, Texture);
    img->proportion = proportion;
    img->resource = NULL;

    self->totalAssets++;
    nApp->prv->loadSDLSurface(img, path);

    if (src) {
        img->src = malloc(sizeof(SDL_Rect));
        nUtil->assertAlloc(img->src);
        *img->src = *src;
    }
    else img->src = NULL;
}


void GxLoadTileset(const char* id, const char* pathF, int start, int end, double proportion) {

    nUtil->assertArgument(start >= 0);

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bPath, 256, pathF, i);
        snprintf(bId, 64, "%s|%d", id, i);
        GxLoadImage(bId, bPath,  NULL, proportion);
    }
}



void GxCreateTiles(const char* image, GxSize size, GxMatrix matrix) {

	char folderId[32], imageId[32];
    nUtil->splitAssetPath(image, folderId, imageId);

    GxFolder* folder = nApp->prv->getFolder(folderId);
    nUtil->assertArgument(folder);
    GxImage* source = GxMapGet(folder->assets, imageId);
    nUtil->assertArgument(source);

    int counter = 0;

	for (int i = 0; i < matrix.nr; i++) {
		for (int j = 0; j < matrix.nc; j++) {
            char bId[64];
            snprintf(bId, 64, "%s|%d", source->id, ++counter);
            GxImage* self = createImage(folder, bId, Opaque);
            self->src = malloc(sizeof(SDL_Rect));
            nUtil->assertAlloc(self->src);
            *self->src = (SDL_Rect) { size.w * j, size.h * i, size.w, size.h };
            self->source = source;
            self->size = size;
		}
	}
}

void GxFolderCreateTilemap(const char* folderName, const char* name, const char* group,
    GxSize size, GxMatrix matrix, int* sequence
){
    GxFolder* folder = nApp->prv->getFolder(folderName);
    nUtil->assertArgument(folder);
    nUtil->assertArgument(folder->status != GxStatusNone);    
    GxImage* self = createImage(folder, name, Palette);
    nUtil->assertAlloc(self);    
    self->size = size;
    self->matrix = matrix;   
    self->children = nArr->create();

    //for jumps in the composed image
    static GxImage sBlank = {.type = Blank};

    for (int i = 0; i < matrix.nr * matrix.nc; i++) {
        GxImage* image = NULL;
        const char* stringImage = NULL;
        char child[64] = {'\0'};

        if (sequence) {
            if (sequence[i] == -1) {
                image = &sBlank;
            }
            else {
                snprintf(child, 64, "%s|%d", group, sequence[i]);
                stringImage = child;
            }
        }
        else {
            stringImage = group;
        }
        image = image ? image : GxMapGet(folder->assets, stringImage);
        nUtil->assertResourceNotFound(image);
        nArr->push(self->children, image, NULL);
    }   
}

GxSize GxImageGetSize_(GxImage* self) {
    return self->size;
}

GxSize GxFolderGetImageSize(const char* path) {
	nUtil->assertArgument(path);
    char folderId[32];
    char imageId[32];
    nUtil->splitAssetPath(path, folderId, imageId);
    GxFolder* folder = nApp->prv->getFolder(folderId);
    nUtil->assertArgument(folder);
    GxImage* image = GxFolderGetImage_(folder, imageId);
    nUtil->assertArgument(image);
    return image->size;
}

const char* GxImageGetId_(GxImage* self) {
	return self->id;
}

void GxImageTextureSetResource_(GxImage* self, void* resource, GxSize* size) {

    self->resource = resource;

    if (self->src) {
        self->size.w = (int)(self->src->w * self->proportion + 0.5);
        self->size.h = (int)(self->src->h * self->proportion + 0.5);
    }
    else {
        self->size.w = (int)(size->w * self->proportion + 0.5);
        self->size.h = (int)(size->h * self->proportion + 0.5);
    }
    folderIncreaseAssetsLoaded(self->folder);
}

static void destroyAnimation(GxAnimation* self) {
    if (self) {
        nArr->destroy(self->images);
        free(self->id);
        free(self);
    }
}

void GxLoadAnimation(const char* id, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous)
{
    nUtil->assertArgument(start >= 0);

    GxFolder* folder = sFolder;
    GxAnimation* self = malloc(sizeof(GxAnimation));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->images = nArr->create();
	self->interval = interval;
    self->continuous = continuous;
    self->quantity = end - start + 1;

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bId, 64, "%s|%d", id, i);
        snprintf(bPath, 256, pathF, i);
        GxLoadImage(bId, bPath, NULL, proportion);
        nArr->push(self->images, GxMapGet(folder->assets, bId), NULL);
	}
    if (!folder->assets) folder->assets = GmCreateMap();
    GxMapSet(folder->assets, self->id, self, (GxDestructor) destroyAnimation);
}

bool GxAnimIsContinous_(GxAnimation* self){
    return self->continuous;
}

Uint32 GxAnimGetInterval_(GxAnimation* self) {
    return self->interval;
}

Uint32 GxAnimGetQuantity_(GxAnimation* self) {
    return self->quantity;
}

GxImage* GxAnimGetImage_(GxAnimation* self, Uint32 index) {
    return nArr->at(self->images, index);
}

const char* GxAnimGetId_(GxAnimation* self) {
    return self->id;
}

static inline void destroySound(GxSound* self) {
    if (self) {
        free(self->id);
        Mix_FreeChunk(self->chunk);
    }
}

void GxSoundSetChunk_(GxSound* self, Mix_Chunk* chunk) {
    self->chunk = chunk;
    folderIncreaseAssetsLoaded(self->folder);
}

void GxLoadChunk(const char* id, const char* path) {

    GxSound* self = malloc(sizeof(GxSound));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = sFolder;
    self->chunk = NULL;
    nApp->prv->loadMixChunk(self, path);
    sFolder->totalAssets++;
    if (!sFolder->assets) sFolder->assets = GmCreateMap();
    GxMapSet(sFolder->assets, self->id, self, destroySound);
}

static inline void GxDestroyMusic_(GxMusic* self) {
    if (self) {
        free(self->id);
        Mix_FreeMusic(self->music);
    }
}

void GxMusicSetMixMusic_(GxMusic* self, Mix_Music* music) {
    self->music = music;
    folderIncreaseAssetsLoaded(self->folder);
}

void GxLoadMusic(const char* id, const char* path) {
    GxMusic* self = malloc(sizeof(GxMusic));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = sFolder;
    self->music = NULL;
    nApp->prv->loadMixMusic(self, path);
    sFolder->totalAssets++;
    if (!sFolder->assets) sFolder->assets = GmCreateMap();
    GxMapSet(sFolder->assets, self->id, self, (GxDestructor) GxDestroyMusic_);
}
