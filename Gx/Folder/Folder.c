#include "../Folder/Folder.h"
#include "../Containers/Map/Map.h"
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
    char* id;
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
static void create(const char* id, void(*loader)(void)) {

    sFolder* self = malloc(sizeof(sFolder));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->status = loader ? nUtil->status->NONE : nUtil->status->READY;
    self->assetsLoaded = 0;
    self->totalAssets = 0;
    self->assets = nMap->create();
    self->loader = loader;
    self->refCounter = 0;
    nApp->p_->addFolder(self);
}

static void pDestroy(sFolder* self) {

    if (self) {
       nMap->destroy(self->assets);
        free(self->id);
        free(self);
    }
}

//accessors and mutators
static char* pId(sFolder* self) {
    return self->id;
}

static bool pHasStatus(sFolder* self, int status) {
    return self->status == status;
}

static sImage* getImage(sFolder* self, const char* id) {
    return nMap->get(self->assets, id);
}

static Mix_Music* getMixMusic(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    sFolder* folder = nApp->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(folder);
    sMusic* asset = nMap->get(folder->assets, nArray->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->music;
}

static Mix_Chunk* getMixChunk(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    sFolder* folder = nApp->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(folder);
    sChunk* asset = nMap->get(folder->assets, nArray->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->chunk;
}

static sAnimation* getAnimation(sFolder* self, const char* id) {
     return nMap->get(self->assets, id);
}

static void iLoadFolder(sFolder* self) {
    if(self->status == nUtil->status->NONE){
        self->status = nUtil->status->LOADING;
        rFolder = self;
        self->loader();
        rFolder = NULL;
    }
}

static int pGetPercLoaded(sFolder* self) {
    return self->totalAssets ? (self->assetsLoaded * 100 / self->totalAssets) : 100;
}

static void iUnloadFolder(sFolder* self) {
    nMap->clean(self->assets);
    self->status = nUtil->status->NONE;
}

static void iIncreaseAssetsLoaded(sFolder* self){
    self->assetsLoaded++;
    if(self->assetsLoaded == self->totalAssets){
        self->status = nUtil->status->READY;
    }
}

static void pIncRefCounter(sFolder* self) {
    if (self->refCounter == 0) iLoadFolder(self);
    self->refCounter++;
}

static void pDecRefCounter(sFolder* self) {
    if(nApp->isRunning()){
        nUtil->assertState(self->refCounter > 0);
        --self->refCounter;
        if (self->refCounter == 0) {
           iUnloadFolder(self);
        }
    }
}

static sImage* iCreateImage(sFolder* folder, const char* name, ImageType type) {
    sImage* self = calloc(1, sizeof(sImage));
    nUtil->assertAlloc(self);
    self->name = nUtil->createString(name);
    self->folder = folder;
    self->type = type;
    nMap->set(folder->assets, name, self, pDestroyImage);
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

static sImage* createText(const char* text, const char* fontName, int size, SDL_Color* color){

    sImage* self = calloc(1, sizeof(sImage));
    nUtil->assertAlloc(self);
    self->type = Text;
    const char* fontPath = nApp->p_->getFontPath(fontName);
   
    sSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nApp->SDLRenderer(), &wsize.w, &wsize.h);
    sSize lsize = nApp->logicalSize();
    size = ((double) size * wsize.w) / lsize.w;

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

static void destroyText(sImage* self) { 
    nUtil->assertArgument(self && self->type == Text);
    pDestroyImage(self);
}

static void loadImage(const char* id, const char* path, sRect* src, double proportion) {

    sFolder* self = rFolder;
    nUtil->assertArgument(self->assets == NULL || nMap->get(self->assets, id) == NULL);
    nUtil->assertState(self->status == nUtil->status->LOADING);
    sImage* img = iCreateImage(self, id, Texture);
    img->proportion = proportion;
    img->resource = NULL;

    self->totalAssets++;
    nApp->p_->loadSDLSurface(img, path);

    if (src) {
        img->src = malloc(sizeof(sRect));
        nUtil->assertAlloc(img->src);
        *img->src = *src;
    }
    else img->src = NULL;
}


static void loadTileset(const char* id, const char* pathF, int start, int end, double proportion) {

    nUtil->assertArgument(start >= 0);

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bPath, 256, pathF, i);
        snprintf(bId, 64, "%s|%d", id, i);
        nFolder->loadImage(bId, bPath,  NULL, proportion);
    }
}

static void createTilesetFromImage(const char* image, sSize size, sMatrix matrix) {

	char folderId[32], imageId[32];
    nUtil->splitAssetPath(image, folderId, imageId);

    sFolder* folder = nApp->getFolder(folderId);
    nUtil->assertArgument(folder);
    sImage* source = nMap->get(folder->assets, imageId);
    nUtil->assertArgument(source);

    int counter = 0;

	for (int i = 0; i < matrix.nr; i++) {
		for (int j = 0; j < matrix.nc; j++) {
            char bId[64];
            snprintf(bId, 64, "%s|%d", source->name, ++counter);
            sImage* self = iCreateImage(folder, bId, Opaque);
            self->src = malloc(sizeof(sRect));
            nUtil->assertAlloc(self->src);
            *self->src = (sRect) { size.w * j, size.h * i, size.w, size.h };
            self->source = source;
            self->size = size;
		}
	}
}

static sFolder* getImageFolder(sImage* self) {
    nUtil->assertArgument(self);
    return self->folder;
}

static const sSize* pGetImageSize(sImage* self) {
    nUtil->assertArgument(self);
    return &self->size;
}

static const sRect* getImageSrc(sImage* self) {
     nUtil->assertArgument(self);
    return self->src;
}

static const char* getImageName(sImage* self) {
    nUtil->assertArgument(self);
    return self->name;
}

static SDL_Texture* getImageSDLTexture(sImage* self) {
    nUtil->assertArgument(self);
	return self->resource;
}

static double getImageProportion(sImage* self) {
     nUtil->assertArgument(self);
     return self->proportion;
}

static void pSetSDLTexture(sImage* self, void* resource, sSize* size) {

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
        nArray->destroy(self->images);
        free(self->name);
        free(self);
    }
}

static void loadAnimation(const char* name, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous)
{
    nUtil->assertArgument(start >= 0);

    sFolder* folder = rFolder;
    sAnimation* self = malloc(sizeof(sAnimation));
    nUtil->assertAlloc(self);
    self->name = nUtil->createString(name);
    self->images = nArray->create();
	self->interval = interval;
    self->continuous = continuous;
    self->quantity = end - start + 1;

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bId, 64, "%s|%d", name, i);
        snprintf(bPath, 256, pathF, i);
        nFolder->loadImage(bId, bPath, NULL, proportion);
        nArray->push(self->images, nMap->get(folder->assets, bId), NULL);
	}
    if (!folder->assets) folder->assets = nMap->create();
    nMap->set(folder->assets, self->name, self, (sDtor) iDestroyAnimation);
}

static const char* getAnimName(sAnimation* self) {
    return self->name;
}

static bool pIsAnimContinuous(sAnimation* self){
    return self->continuous;
}

static Uint32 pGetAnimInterval(sAnimation* self) {
    return self->interval;
}

static Uint32 pGetAnimQuantity(sAnimation* self) {
    return self->quantity;
}

static sImage* pGetAnimImage(sAnimation* self, Uint32 index) {
    return nArray->at(self->images, index);
}


static void iDestroyChunk(sChunk* self) {
    if (self) {
        free(self->id);
        Mix_FreeChunk(self->chunk);
    }
}

static void loadChunk(const char* id, const char* path) {

    sChunk* self = malloc(sizeof(sChunk));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = rFolder;
    self->chunk = NULL;
    nApp->p_->loadMixChunk(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = nMap->create();
    nMap->set(rFolder->assets, self->id, self, iDestroyChunk);
}

static void pSetMixChunk(sChunk* self, Mix_Chunk* chunk) {
    self->chunk = chunk;
    iIncreaseAssetsLoaded(self->folder);
}

static void iDestroyMusic(sMusic* self) {
    if (self) {
        free(self->id);
        Mix_FreeMusic(self->music);
    }
}

static void loadMusic(const char* id, const char* path) {
    sMusic* self = malloc(sizeof(sMusic));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = rFolder;
    self->music = NULL;
    nApp->p_->loadMixMusic(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = nMap->create();
    nMap->set(rFolder->assets, self->id, self, (sDtor) iDestroyMusic);
}

static void pSetMixMusic(sMusic* self, Mix_Music* music) {
    self->music = music;
    iIncreaseAssetsLoaded(self->folder);
}


static void pRenderImage(sImage* self, sRect* target, 
    double angle, SDL_RendererFlip orientation, Uint8 opacity) 
{
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
            SDL_Renderer* renderer = nApp->SDLRenderer();
            sRect* dst = ( self->type == Text ? 
                nApp->calcLabelDest(target, &(sRect){0}) 
                : nApp->calcDest(target, &(sRect){0, 0, 0, 0})
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

const struct sFolderNamespace* const nFolder = &(struct sFolderNamespace){

    .create = create,
    .getMixMusic = getMixMusic,
    .getMixChunk = getMixChunk,
    .getImage = getImage,
    .getAnimation = getAnimation,
    .loadImage = loadImage,
    .loadTileset = loadTileset,
    .createTilesetFromImage = createTilesetFromImage,    
    .loadAnimation = loadAnimation,
    .loadChunk = loadChunk,
    .loadMusic = loadMusic,    

    .img = &(const struct sImageNamespace) {
        .name = getImageName,
        .folder = getImageFolder,
        .size = pGetImageSize,
        .SDLTexture = getImageSDLTexture,
        .src = getImageSrc,
        .proportion = getImageProportion,
        .createText = createText,
        .render = pRenderImage,
        .destroyText = destroyText,
    },

    .anim = &(const struct sAnimationNamespace) {
        .name = getAnimName,
        .interval = pGetAnimInterval,
        .quantity = pGetAnimQuantity,
        .isContinuous = pIsAnimContinuous,
        .getImage = pGetAnimImage,
    },

    .p_ = &(struct sFolderPrivateNamespace){
       .destroy = pDestroy,
       .hasStatus = pHasStatus,
       .id = pId,
       .getPercLoaded = pGetPercLoaded,
       .incRefCounter = pIncRefCounter,
       .decRefCounter = pDecRefCounter,          
       
       .setMixChunk = pSetMixChunk,
       .setMixMusic = pSetMixMusic,
       .setSDLTexture = pSetSDLTexture,                                  
    },    
};