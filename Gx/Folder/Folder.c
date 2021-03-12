#include "../Folder/Folder.h"
#include "../Containers/Map/Map.h"
#include "../Containers/Array/Array.h"
#include "../Util/Util.h"
#include "../Scene/Scene.h"
#include "../Element/Element.h"
#include "../App/App.h"
#include <stdbool.h>
#include <string.h>
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <stdio.h>


typedef struct sFolder {
    char* name;
	int status;   
	sMap* assets;
    //...
    int assetsLoaded;
    int totalAssets;
    
    //...
    int refCounter;

    //...der
	void(*loader)(void);
} sFolder;

static sFolder* rFolder = NULL;

typedef enum ImageType {
	Texture,
	Text,
	Opaque,	
} ImageType;

typedef struct sImage {

    char* name;
    sFolder* folder;
	ImageType type;
    sSize size;

    //...
    SDL_Texture* resource;
    sRect* src;
    double proportion;

    //... opaque
    sImage* source; 

} sImage;


typedef struct sAnimation {
    char* name;
    sArray* images;
    Uint32 interval;
    Uint32 quantity;
    bool continuous;
} sAnimation;


typedef struct sChunk {
    char* id;
    sFolder* folder;
    Mix_Chunk* chunk;
} sChunk;

typedef struct sMusic {
    char* id;
    sFolder* folder;
    Mix_Music* music;
} sMusic;

//
static void pDestroyImage(sImage* self);

//...
void nFolderCreate(const char* name, void(*loader)(void)) {

    sFolder* self = malloc(sizeof(sFolder));
   nUtilAssertAlloc(self);
    self->name = nUtilCreateString(name);
    self->status = loader ? nUTIL_STATUS_NONE : nUTIL_STATUS_READY;
    self->assetsLoaded = 0;
    self->totalAssets = 0;
    self->assets = nMapCreate();
    self->loader = loader;
    self->refCounter = 0;
    nAppAddFolder_(self);
}

void nFolderDestroy_(sFolder* self) {

    if (self) {
       nMapDestroy(self->assets);
        free(self->name);
        free(self);
    }
}

//accessors and mutators
char* nFolderName(sFolder* self) {
    return self->name;
}

bool nFolderHasStatus(sFolder* self, int status) {
    return self->status == status;
}

sImage* nFolderGetImage(sFolder* self, const char* id) {
    return nMapGet(self->assets, id);
}

Mix_Music* nFolderGetMixMusic(const char* path) {
	sArray* tokens = nAppTokenize(path, "/");
   nUtilAssertArgument(nArraySize(tokens) == 2);
    sFolder* folder = nAppGetFolder(nArrayAt(tokens, 0));
   nUtilAssertArgument(folder);
    sMusic* asset = nMapGet(folder->assets, nArrayAt(tokens, 1));
   nUtilAssertArgument(asset);
    return asset->music;
}

Mix_Chunk* nFolderGetMixChunk(const char* path) {
	sArray* tokens = nAppTokenize(path, "/");
   nUtilAssertArgument(nArraySize(tokens) == 2);
    sFolder* folder = nAppGetFolder(nArrayAt(tokens, 0));
   nUtilAssertArgument(folder);
    sChunk* asset = nMapGet(folder->assets, nArrayAt(tokens, 1));
   nUtilAssertArgument(asset);
    return asset->chunk;
}

sAnimation* nFolderGetAnimation(sFolder* self, const char* id) {
     return nMapGet(self->assets, id);
}

static void iLoadFolder(sFolder* self) {
    if(self->status == nUTIL_STATUS_NONE){
        self->status = nUTIL_STATUS_LOADING;
        rFolder = self;
        self->loader();
        rFolder = NULL;
    }
}

int nFolderGetPercLoaded(sFolder* self) {
    return self->totalAssets ? (self->assetsLoaded * 100 / self->totalAssets) : 100;
}

static void iUnloadFolder(sFolder* self) {
    nMapClean(self->assets);
    self->status = nUTIL_STATUS_NONE;
}

static void iIncreaseAssetsLoaded(sFolder* self){
    self->assetsLoaded++;
    if(self->assetsLoaded == self->totalAssets){
        self->status = nUTIL_STATUS_READY;
    }
}

void nFolderIncRefCounter_(sFolder* self) {
    if (self->refCounter == 0) iLoadFolder(self);
    self->refCounter++;
}

void nFolderDecRefCounter_(sFolder* self) {
    if(nAppIsRunning()){
        nUtilAssertState(self->refCounter > 0);
        --self->refCounter;
        if (self->refCounter == 0) {
           iUnloadFolder(self);
        }
    }
}

static sImage* iCreateImage(sFolder* folder, const char* name, ImageType type) {
    sImage* self = calloc(1, sizeof(sImage));
   nUtilAssertAlloc(self);
    self->name = nUtilCreateString(name);
    self->folder = folder;
    self->type = type;
    nMapSet(folder->assets, name, self, pDestroyImage);
    return self;
}

static void pDestroyImage(sImage* self) {
    if (self) {
        if (self->resource){
            SDL_DestroyTexture(self->resource);
        }       
        free(self->src);
        free(self->name);
        free(self);
    }
}

sImage* nImageCreateText(const char* text, const char* fontName, int size, SDL_Color* color){

    sImage* self = calloc(1, sizeof(sImage));   
    
    nUtilAssertAlloc(self);
    self->type = Text;
      
    sSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nAppSDLRenderer(), &wsize.w, &wsize.h);
    sSize lsize = nAppLogicalSize();
    size = ((double) size * wsize.w) / lsize.w;    

    TTF_Font* font = nAppLoadFont_(fontName, size);
   
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, *color);
    if (!surface){
        nAppRuntimeError(SDL_GetError());
    }
    self->size.w = surface->w;
    self->size.h = surface->h;
    self->resource = SDL_CreateTextureFromSurface(nAppSDLRenderer(), surface);
    SDL_FreeSurface(surface);
    return self;
}

void nImageDestroyText(sImage* self) { 
   nUtilAssertArgument(self && self->type == Text);
    pDestroyImage(self);
}

void nFolderLoadImage(const char* id, const char* path, sRect* src, double proportion) {

    sFolder* self = rFolder;
   nUtilAssertArgument(self->assets == NULL || nMapGet(self->assets, id) == NULL);
    nUtilAssertState(self->status == nUTIL_STATUS_LOADING);
    sImage* img = iCreateImage(self, id, Texture);
    img->proportion = proportion;
    img->resource = NULL;

    self->totalAssets++;
    nAppLoadSDLSurface_(img, path);

    if (src) {
        img->src = malloc(sizeof(sRect));
       nUtilAssertAlloc(img->src);
        *img->src = *src;
    }
    else img->src = NULL;
}


void nFolderLoadTileset(const char* id, const char* pathF, int start, int end, double proportion) {

   nUtilAssertArgument(start >= 0);

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bPath, 256, pathF, i);
        snprintf(bId, 64, "%s|%d", id, i);
        nFolderLoadImage(bId, bPath,  NULL, proportion);
    }
}

void nFolderCreateTilesetFromImage(const char* image, sSize size, sMatrix matrix) {

	char folderId[32], imageId[32];
    nUtilSplitAssetPath(image, folderId, imageId);

    sFolder* folder = nAppGetFolder(folderId);
   nUtilAssertArgument(folder);
    sImage* source = nMapGet(folder->assets, imageId);
   nUtilAssertArgument(source);

    int counter = 0;

	for (int i = 0; i < matrix.nr; i++) {
		for (int j = 0; j < matrix.nc; j++) {
            char bId[64];
            snprintf(bId, 64, "%s|%d", source->name, ++counter);
            sImage* self = iCreateImage(folder, bId, Opaque);
            self->src = malloc(sizeof(sRect));
           nUtilAssertAlloc(self->src);
            *self->src = (sRect) { size.w * j, size.h * i, size.w, size.h };
            self->source = source;
            self->size = size;
		}
	}
}

sFolder* nImageFolder(sImage* self) {
   nUtilAssertArgument(self);
    return self->folder;
}

const sSize* nImageSize(sImage* self) {
   nUtilAssertArgument(self);
    return &self->size;
}

const sRect* nImageSrc(sImage* self) {
    nUtilAssertArgument(self);
    return self->src;
}

const char* nImageName(sImage* self) {
   nUtilAssertArgument(self);
    return self->name;
}

SDL_Texture* nImageSDLTexture(sImage* self) {
   nUtilAssertArgument(self);
	return self->resource;
}

double nImageProportion(sImage* self) {
    nUtilAssertArgument(self);
     return self->proportion;
}

void nFolderSetSDLTexture_(sImage* self, void* resource, sSize* size) {

    self->resource = resource;

    if (self->src) {
        self->size.w = (int)(self->src->w * self->proportion + 0.5);
        self->size.h = (int)(self->src->h * self->proportion + 0.5);
    }
    else {
        self->size.w = (int)(size->w * self->proportion + 0.5);
        self->size.h = (int)(size->h * self->proportion + 0.5);
    }
    iIncreaseAssetsLoaded(self->folder);
}

static void iDestroyAnimation(sAnimation* self) {
    if (self) {
        nArrayDestroy(self->images);
        free(self->name);
        free(self);
    }
}

void nFolderLoadAnimation(const char* name, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous)
{
   nUtilAssertArgument(start >= 0);

    sFolder* folder = rFolder;
    sAnimation* self = malloc(sizeof(sAnimation));
   nUtilAssertAlloc(self);
    self->name = nUtilCreateString(name);
    self->images = nArrayCreate();
	self->interval = (Uint32) interval;
    self->continuous = continuous;
    self->quantity = end - start + 1;

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bId, 64, "%s|%d", name, i);
        snprintf(bPath, 256, pathF, i);
        nFolderLoadImage(bId, bPath, NULL, proportion);
        nArrayPush(self->images, nMapGet(folder->assets, bId), NULL);
	}
    if (!folder->assets) folder->assets = nMapCreate();
    nMapSet(folder->assets, self->name, self, (sDtor) iDestroyAnimation);
}

const char* nAnimName(sAnimation* self) {
    return self->name;
}

bool nAnimIsContinuous(sAnimation* self){
    return self->continuous;
}

Uint32 nAnimInterval(sAnimation* self) {
    return self->interval;
}

Uint32 nAnimQuantity(sAnimation* self) {
    return self->quantity;
}

sImage* nAnimGetImage(sAnimation* self, Uint32 index) {
    return nArrayAt(self->images, index);
}

static void iDestroyChunk(sChunk* self) {
    if (self) {
        free(self->id);
        Mix_FreeChunk(self->chunk);
    }
}

void nFolderLoadChunk(const char* id, const char* path) {

    sChunk* self = malloc(sizeof(sChunk));
   nUtilAssertAlloc(self);
    self->id = nUtilCreateString(id);
    self->folder = rFolder;
    self->chunk = NULL;
    nAppLoadMixChunk_(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = nMapCreate();
    nMapSet(rFolder->assets, self->id, self, iDestroyChunk);
}

void nFolderSetMixChunk_(sChunk* self, Mix_Chunk* chunk) {
    self->chunk = chunk;
    iIncreaseAssetsLoaded(self->folder);
}

static void iDestroyMusic(sMusic* self) {
    if (self) {
        free(self->id);
        Mix_FreeMusic(self->music);
    }
}

void nFolderLoadMusic(const char* id, const char* path) {
    sMusic* self = malloc(sizeof(sMusic));
   nUtilAssertAlloc(self);
    self->id = nUtilCreateString(id);
    self->folder = rFolder;
    self->music = NULL;
    nAppLoadMixMusic_(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = nMapCreate();
    nMapSet(rFolder->assets, self->id, self, (sDtor) iDestroyMusic);
}

void nFolderSetMixMusic_(sMusic* self, Mix_Music* music) {
    self->music = music;
    iIncreaseAssetsLoaded(self->folder);
}


void nImageRender(sImage* self, sRect* target, 
    double angle, SDL_RendererFlip orientation, Uint8 opacity) 
{
    if (self->type == Texture || self->type == Opaque || self->type == Text){
        void* resource = NULL;
        sRect* src = NULL;
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
            SDL_Renderer* renderer = nAppSDLRenderer();
            sRect* dst = ( self->type == Text ? 
                nAppCalcLabelDest(target, &(sRect){0}) 
                : nAppCalcDest(target, &(sRect){0, 0, 0, 0})
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
}

