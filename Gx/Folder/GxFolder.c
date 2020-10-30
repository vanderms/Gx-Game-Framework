#include "../Folder/GxFolder.h"
#include "../Map/GxMap.h"
#include "../Utilities/GxUtil.h"
#include "../Scene/GxScene.h"
#include "../Element/GxElement.h"
#include "../App/GxApp.h"
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

typedef enum ImageType{
	Fill,
	Draw,
	Texture,
	Text,
	Palette,
	Opaque,
	Blank,
} ImageType;

typedef struct GxImage {

    char* id;
    GxFolder* folder;
	ImageType type;
    GxSize size;

    //texture
    SDL_Texture* resource;
    SDL_Rect* src;
    double proportion;

    //opaque
    GxImage* source;

     //palette
    GxArray* children;
    GxMatrix matrix;
} GxImage;


typedef struct GxAnimation {
    char* id;
    GxArray* images;
    Uint32 interval;
    Uint32 quantity;
    bool continuous;
} GxAnimation;

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
    GxAssertAllocationFailure(self);
    self->id = GmCreateString(id);
    self->status = loader ? GxStatusNone : GxStatusReady;
    self->assetsLoaded = 0;
    self->totalAssets = 0;
    self->assets = GmCreateMap();
    self->loader = loader;
    self->refCounter = 0;
    GxAddFolder_(self);
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
	GxArray* tokens = GxTokenize(path, "/");
    GxAssertInvalidArgument(GxArraySize(tokens) == 2);
    GxFolder* folder = GxGetFolder_(GxArrayAt(tokens, 0));
    GxAssertInvalidArgument(folder);
    GxMusic* asset = GxMapGet(folder->assets, GxArrayAt(tokens, 1));
    GxAssertInvalidArgument(asset);
    return asset->music;
}

Mix_Chunk* GxFolderGetChunk(const char* path) {
	GxArray* tokens = GxTokenize(path, "/");
    GxAssertInvalidArgument(GxArraySize(tokens) == 2);
    GxFolder* folder = GxGetFolder_(GxArrayAt(tokens, 0));
    GxAssertInvalidArgument(folder);
    GxSound* asset = GxMapGet(folder->assets, GxArrayAt(tokens, 1));
    GxAssertInvalidArgument(asset);
    return asset->chunk;
}

SDL_Texture* GxFolderGetTexture(const char* path) {
	GxArray* tokens = GxTokenize(path, "/");
    GxAssertInvalidArgument(GxArraySize(tokens) == 2);
    GxFolder* folder = GxGetFolder_(GxArrayAt(tokens, 0));
    GxAssertInvalidArgument(folder);
    GxImage* asset = GxMapGet(folder->assets, GxArrayAt(tokens, 1));
    GxAssertInvalidArgument(asset);
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
    if(GxAppIsRunning_()){
        GxAssertInvalidOperation(self->refCounter > 0);
        --self->refCounter;
        if (self->refCounter == 0) {
           folderUnload(self);
        }
    }
}

static GxImage* createImage(GxFolder* folder, const char* id, ImageType type) {
    GxImage* self = calloc(1, sizeof(GxImage));
    GxAssertAllocationFailure(self);
    self->id = GmCreateString(id);
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
        GxDestroyArray(self->children);
        free(self->src);
        free(self->id);
        free(self);
    }
}

GxImage* GxImageCreateText_(const char* text, const char* fontName, int size, SDL_Color* color){

    GxImage* self = calloc(1, sizeof(GxImage));
    GxAssertAllocationFailure(self);
    self->type = Texture;
    const char* fontPath = GxGetFontPath_(fontName);
    TTF_Font* font = TTF_OpenFont(fontPath, size);
    if(!font){
        GxFatalError(TTF_GetError());
    }
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, *color);
    if (!surface){
        GxFatalError(TTF_GetError());
    }
    self->size.w = surface->w;
    self->size.h = surface->h;
    self->resource = SDL_CreateTextureFromSurface(GxGetSDLRenderer(), surface);
    SDL_FreeSurface(surface);
    return self;
}

void GxLoadImage(const char* id, const char* path, SDL_Rect* src, double proportion) {

    GxFolder* self = sFolder;
    GxAssertInvalidArgument(self->assets == NULL || GxMapGet(self->assets, id) == NULL);
    GxAssertInvalidOperation(self->status == GxStatusLoading);
    GxImage* img = createImage(self, id, Texture);
    img->proportion = proportion;
    img->resource = NULL;

    self->totalAssets++;
    GxPushTextureToLoad_(img, path);

    if (src) {
        img->src = malloc(sizeof(SDL_Rect));
        GxAssertAllocationFailure(img->src);
        *img->src = *src;
    }
    else img->src = NULL;
}


void GxLoadTileset(const char* id, const char* pathF, int start, int end, double proportion) {

    GxAssertInvalidArgument(start >= 0);

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bPath, 256, pathF, i);
        snprintf(bId, 64, "%s|%d", id, i);
        GxLoadImage(bId, bPath,  NULL, proportion);
    }
}

void GxSplitAssetPath_(const char* path, char* folder, char* asset) {
    char clone[64];
    GxCloneString(path, clone, 64);
    char* div = strstr(clone, "/");
    GxAssertInvalidArgument(div);
    div[0] = '\0';
    GxCloneString(clone, folder, 32);
    GxCloneString(div + 1, asset, 32);
}


void GxCreateTiles(const char* image, GxSize size, GxMatrix matrix) {

	char folderId[32], imageId[32];
    GxSplitAssetPath_(image, folderId, imageId);

    GxFolder* folder = GxGetFolder_(folderId);
    GxAssertInvalidArgument(folder);
    GxImage* source = GxMapGet(folder->assets, imageId);
    GxAssertInvalidArgument(source);

    int counter = 0;

	for (int i = 0; i < matrix.nr; i++) {
		for (int j = 0; j < matrix.nc; j++) {
            char bId[64];
            snprintf(bId, 64, "%s|%d", source->id, ++counter);
            GxImage* self = createImage(folder, bId, Opaque);
            self->src = malloc(sizeof(SDL_Rect));
            GxAssertAllocationFailure(self->src);
            *self->src = (SDL_Rect) { size.w * j, size.h * i, size.w, size.h };
            self->source = source;
            self->size = size;
		}
	}
}

GxImage* GxCreateTilePalette_(GxFolder* folder, const char* group,
    GxSize size, GxMatrix matrix, int* sequence) {

    if(folder->status == GxStatusNone) folderLoad_(folder);
    GxImage* self = calloc(1, sizeof(GxImage));
    GxAssertAllocationFailure(self);
    self->type = Palette;
    self->size = size;
    self->matrix = matrix;
    self->folder = folder;
    self->children = GxCreateArray();

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
        GxAssertNotFoundAsset(image);
        GxArrayPush(self->children, image, NULL);
    }
    return self;
}

GxSize GxImageGetSize_(GxImage* self) {
    return self->size;
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

void GxImageRender_(GxImage* self, SDL_Rect* target, double angle, SDL_RendererFlip orientation) {

    if (self->type == Texture || self->type == Opaque){
        void* resource = NULL;
        void* src = NULL;
        if (self->type == Texture) {
            resource = self->resource;
            src = self->src;
        }
        else { // === Opaque
            resource =  self->source->resource;
            src = self->src;
        }
	    if (resource) {
            SDL_Renderer* renderer = GxGetSDLRenderer();
            if ((angle <= -1.0 || angle >= 1.0) || orientation != SDL_FLIP_NONE) {
                SDL_RenderCopyEx(renderer, resource, src, target, angle, NULL, orientation);
            }
            else {
                SDL_RenderCopy(renderer, resource, src, target);
            }
        }
	}
	else if (self->type == Palette) {
		GxImageRenderTilePalette_(self, target);
	}
}

void GxImageRenderTilePalette_(GxImage* self, SDL_Rect* target) {

    if (self->folder->status != GxStatusReady){ return; }

    int w = (self->size.w / self->matrix.nc);
    int h = (self->size.h / self->matrix.nr);

    int rowStart = 0;
    int rowEnd = self->matrix.nr;
    int columnStart = 0;
    int columnEnd =  self->matrix.nc;

    GxSize windowSize = GxGetWindowSize();

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
            GxImage* child = GxArrayAt(self->children, index);
            if(child->type == Blank){ continue; }
            int x = (target->x + columns * w);

            //calc child pos
            SDL_Rect pos = {
                .x  = x - ((child->size.w - w) / 2), //...xcenter texture
                .y = y - ((child->size.h - h) / 2), //... ycenter texture
                .w = child->size.w,
                .h = child->size.h
            };

            GxImageRender_(child, &pos, 0.0, SDL_FLIP_NONE);
        }
    }
}

static void destroyAnimation(GxAnimation* self) {
    if (self) {
        GxDestroyArray(self->images);
        free(self->id);
        free(self);
    }
}

void GxLoadAnimation(const char* id, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous)
{
    GxAssertInvalidArgument(start >= 0);

    GxFolder* folder = sFolder;
    GxAnimation* self = malloc(sizeof(GxAnimation));
    GxAssertAllocationFailure(self);
    self->id = GmCreateString(id);
    self->images = GxCreateArray();
	self->interval = interval;
    self->continuous = continuous;
    self->quantity = end - start + 1;

    for (int i = start; i <= end; i++) {
        char bId[64];
        char bPath[256];
        snprintf(bId, 64, "%s|%d", id, i);
        snprintf(bPath, 256, pathF, i);
        GxLoadImage(bId, bPath, NULL, proportion);
        GxArrayPush(self->images, GxMapGet(folder->assets, bId), NULL);
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
    return GxArrayAt(self->images, index);
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
    GxAssertAllocationFailure(self);
    self->id = GmCreateString(id);
    self->folder = sFolder;
    self->chunk = NULL;
    GxPushChunkToLoad_(self, path);
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
    GxAssertAllocationFailure(self);
    self->id = GmCreateString(id);
    self->folder = sFolder;
    self->music = NULL;
    gxPushMusicToLoad_(self, path);
    sFolder->totalAssets++;
    if (!sFolder->assets) sFolder->assets = GmCreateMap();
    GxMapSet(sFolder->assets, self->id, self, (GxDestructor) GxDestroyMusic_);
}
