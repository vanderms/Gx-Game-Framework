#include "../Folder/Folder.h"
#include "../Private/GxGraphicAssets.h"
#include "../Map/GxMap.h"
#include "../Utilities/Util.h"
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
	GxMap* assets;
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
	Palette,
	Opaque,
	Blank,
} ImageType;

typedef struct sImage {

    char* id;
    sFolder* folder;
	ImageType type;
    sSize size;

    //...
    SDL_Texture* resource;
    SDL_Rect* src;
    double proportion;

    //... opaque
    sImage* source;

     //... palette
    sArray* children;
    sMatrix matrix;

} sImage;


typedef struct sAnimation {
    char* id;
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



//...
static void create(const char* id, void(*loader)(void)) {

    sFolder* self = malloc(sizeof(sFolder));
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

static void pDestroy(sFolder* self) {

    if (self) {
        GxDestroyMap(self->assets);
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

static sImage* pGetImage(sFolder* self, const char* id) {
    return GxMapGet(self->assets, id);
}

static Mix_Music* getMixMusic(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    sFolder* folder = nApp->prv->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(folder);
    sMusic* asset = GxMapGet(folder->assets, nArray->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->music;
}

static Mix_Chunk* getMixChunk(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    sFolder* folder = nApp->prv->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(folder);
    sChunk* asset = GxMapGet(folder->assets, nArray->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->chunk;
}

static SDL_Texture* getSDLTexture(const char* path) {
	sArray* tokens = nApp->tokenize(path, "/");
    nUtil->assertArgument(nArray->size(tokens) == 2);
    sFolder* folder = nApp->prv->getFolder(nArray->at(tokens, 0));
    nUtil->assertArgument(folder);
    sImage* asset = GxMapGet(folder->assets, nArray->at(tokens, 1));
    nUtil->assertArgument(asset);
    return asset->resource;
}

static sAnimation* pGetAnimation(sFolder* self, const char* id) {
     return GxMapGet(self->assets, id);
}

static void iLoadFolder(sFolder* self) {
    if(self->status == GxStatusNone){
        self->status = GxStatusLoading;
        rFolder = self;
        self->loader();
        rFolder = NULL;
    }
}

static int pGetPercLoaded(sFolder* self) {
    return self->totalAssets ? (self->assetsLoaded * 100 / self->totalAssets) : 100;
}

static void iUnloadFolder(sFolder* self) {
    GxMapClean(self->assets);
    self->status = GxStatusNone;
}

static void iIncreaseAssetsLoaded(sFolder* self){
    self->assetsLoaded++;
    if(self->assetsLoaded == self->totalAssets){
        self->status = GxStatusReady;
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

static sImage* iCreateImage(sFolder* folder, const char* id, ImageType type) {
    sImage* self = calloc(1, sizeof(sImage));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->folder = folder;
    self->type = type;
    GxMapSet(folder->assets, id, self, nFolder->p->destroyImage);
    return self;
}

static void pDestroyImage(sImage* self) {
    if (self) {
        if (self->resource){
            SDL_DestroyTexture(self->resource);
        }
        nArray->destroy(self->children);
        free(self->src);
        free(self->id);
        free(self);
    }
}

static sImage* pCreateText(const char* text, const char* fontName, int size, SDL_Color* color){

    sImage* self = calloc(1, sizeof(sImage));
    nUtil->assertAlloc(self);
    self->type = Text;
    const char* fontPath = nApp->prv->getFontPath(fontName);
   
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

static void loadImage(const char* id, const char* path, SDL_Rect* src, double proportion) {

    sFolder* self = rFolder;
    nUtil->assertArgument(self->assets == NULL || GxMapGet(self->assets, id) == NULL);
    nUtil->assertState(self->status == GxStatusLoading);
    sImage* img = iCreateImage(self, id, Texture);
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

    sFolder* folder = nApp->prv->getFolder(folderId);
    nUtil->assertArgument(folder);
    sImage* source = GxMapGet(folder->assets, imageId);
    nUtil->assertArgument(source);

    int counter = 0;

	for (int i = 0; i < matrix.nr; i++) {
		for (int j = 0; j < matrix.nc; j++) {
            char bId[64];
            snprintf(bId, 64, "%s|%d", source->id, ++counter);
            sImage* self = iCreateImage(folder, bId, Opaque);
            self->src = malloc(sizeof(SDL_Rect));
            nUtil->assertAlloc(self->src);
            *self->src = (SDL_Rect) { size.w * j, size.h * i, size.w, size.h };
            self->source = source;
            self->size = size;
		}
	}
}

static void createTilemap(const char* folderName, const char* name, const char* group,
    sSize size, sMatrix matrix, int* sequence
){
    sFolder* folder = nApp->prv->getFolder(folderName);
    nUtil->assertArgument(folder);
    nUtil->assertArgument(folder->status != GxStatusNone);    
    sImage* self = iCreateImage(folder, name, Palette);
    nUtil->assertAlloc(self);    
    self->size = size;
    self->matrix = matrix;   
    self->children = nArray->create();

    //for jumps in the composed image
    static sImage sBlank = {.type = Blank};

    for (int i = 0; i < matrix.nr * matrix.nc; i++) {
        sImage* image = NULL;
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
        nArray->push(self->children, image, NULL);
    }   
}

static sSize pGetImageSize(sImage* self) {
    return self->size;
}

static sSize getImageSize(const char* path) {
	nUtil->assertArgument(path);
    char folderId[32];
    char imageId[32];
    nUtil->splitAssetPath(path, folderId, imageId);
    sFolder* folder = nApp->prv->getFolder(folderId);
    nUtil->assertArgument(folder);
    sImage* image = nFolder->p->getImage(folder, imageId);
    nUtil->assertArgument(image);
    return image->size;
}

static const char* pGetImageId(sImage* self) {
	return self->id;
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
        free(self->id);
        free(self);
    }
}

static void loadAnimation(const char* id, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous)
{
    nUtil->assertArgument(start >= 0);

    sFolder* folder = rFolder;
    sAnimation* self = malloc(sizeof(sAnimation));
    nUtil->assertAlloc(self);
    self->id = nUtil->createString(id);
    self->images = nArray->create();
	self->interval = interval;
    self->continuous = continuous;
    self->quantity = end - start + 1;

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bId, 64, "%s|%d", id, i);
        snprintf(bPath, 256, pathF, i);
        nFolder->loadImage(bId, bPath, NULL, proportion);
        nArray->push(self->images, GxMapGet(folder->assets, bId), NULL);
	}
    if (!folder->assets) folder->assets = GmCreateMap();
    GxMapSet(folder->assets, self->id, self, (sDtor) iDestroyAnimation);
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

static const char* pGetAnimId(sAnimation* self) {
    return self->id;
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
    nApp->prv->loadMixChunk(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = GmCreateMap();
    GxMapSet(rFolder->assets, self->id, self, iDestroyChunk);
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
    nApp->prv->loadMixMusic(self, path);
    rFolder->totalAssets++;
    if (!rFolder->assets) rFolder->assets = GmCreateMap();
    GxMapSet(rFolder->assets, self->id, self, (sDtor) iDestroyMusic);
}

static void pSetMixMusic(sMusic* self, Mix_Music* music) {
    self->music = music;
    iIncreaseAssetsLoaded(self->folder);
}


static void pRenderImage(sImage* self, SDL_Rect* target, 
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
            SDL_Rect* dst = ( self->type == Text ? 
                nApp->calcLabelDest(target, &(SDL_Rect){0}) 
                : nApp->calcDest(target, &(SDL_Rect){0, 0, 0, 0})
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
		nFolder->p->renderTilePallete(self, target, opacity);
	}
}

static void pRenderTilePallete(sImage* self, SDL_Rect* target, Uint8 opacity) {
	
    if (!nFolder->p->hasStatus(self->folder, GxStatusReady)){ return; }

    int w = (self->size.w / self->matrix.nc);
    int h = (self->size.h / self->matrix.nr);

    int rowStart = 0;
    int rowEnd = self->matrix.nr;
    int columnStart = 0;
    int columnEnd =  self->matrix.nc;

    sSize windowSize = nApp->logicalSize();

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
            sImage* child = nArray->at(self->children, index);
            if(child->type == Blank){ continue; }
            int x = (target->x + columns * w);

            //calc child pos
            SDL_Rect pos = {
                .x  = x - ((child->size.w - w) / 2), //...xcenter texture
                .y = y - ((child->size.h - h) / 2), //... ycenter texture
                .w = child->size.w,
                .h = child->size.h
            };           
            nFolder->p->renderImage(child, &pos, 0.0, SDL_FLIP_NONE, opacity);
        }
    }
}


const struct sFolderNamespace* nFolder = &(struct sFolderNamespace){

    .create = create,
    .getMixMusic = getMixMusic,
    .getMixChunk = getMixChunk,
    .getSDLTexture = getSDLTexture,
    .loadImage = loadImage,
    .loadTileset = loadTileset,
    .createTilesetFromImage = createTilesetFromImage,
    .createTilemap = createTilemap,
    .getImageSize = getImageSize,
    .loadAnimation = loadAnimation,
    .loadChunk = loadChunk,
    .loadMusic = loadMusic,    

    .p = &(struct sFolderPrivateNamespace){
       .destroy = pDestroy,
       .hasStatus = pHasStatus,
       .id = pId,
       .getPercLoaded = pGetPercLoaded,
       .incRefCounter = pIncRefCounter,
       .decRefCounter = pDecRefCounter,
       .getImage = pGetImage,
       .getAnimation = pGetAnimation,
       
       .setMixChunk = pSetMixChunk,
       .setMixMusic = pSetMixMusic,
       .setSDLTexture = pSetSDLTexture,
       
       .destroyImage = pDestroyImage,
       .getImageId = pGetImageId,
       .getImageSize = pGetImageSize,
       .createText = pCreateText,
       .renderImage = pRenderImage,
       .renderTilePallete = pRenderTilePallete,
       
       .getAnimId = pGetAnimId,
       .isAnimContinuous = pIsAnimContinuous,
       .getAnimInterval = pGetAnimInterval,
       .getAnimQuantity = pGetAnimQuantity,
       .getAnimImage = pGetAnimImage,
    },    
};