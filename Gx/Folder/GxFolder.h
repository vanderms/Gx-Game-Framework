#ifndef GX_MODULE_H
#define GX_MODULE_H
#include "../Utilities/GxUtil.h"
#include "SDL.h"
#include "SDL_mixer.h"

//forward declaration
typedef struct GxFolder GxFolder;

void GxCreateFolder(const char* id, void(*loader)(void));

void GxDestroyFolder_(GxFolder* self);

//accessors and mutators
char* GxFolderGetId_(GxFolder* self);

bool GxFolderHasStatus_(GxFolder* self, int status);

GxImage* GxFolderGetImage_(GxFolder* self, const char* id);

Mix_Music* GxFolderGetMusic(const char* path);

Mix_Chunk* GxFolderGetChunk(const char* path);

SDL_Texture* GxFolderGetTexture(const char* path);

GxAnimation* GxFolderGetAnimation_(GxFolder* self, const char* id);

int GXFolderGetPercLoaded_(GxFolder* self);

void GxFolderIncRefCounter_(GxFolder* self);

void GxFolderDecRefCounter_(GxFolder* self);


GxImage* GxImageCreateText_(const char* text, const char* font, int size, SDL_Color* color);

void GxLoadImage(const char* id, const char* path, SDL_Rect* src, double proportion);

void GxDestroyImage_(GxImage* self);

void GxLoadTileset(const char* id, const char* pathF, int start, int end, double proportion);

void GxSplitAssetPath_(const char* path, char* folder, char* asset);

void GxCreateTiles(const char* image, GxSize size, GxMatrix matrix);

GxImage* GxCreateTilePalette_(GxFolder* folder,
    const char* group, GxSize size, GxMatrix matrix, int* sequence
);

GxSize GxImageGetSize_(GxImage* self);
GxSize GxFolderGetImageSize(const char* path);

const char* GxImageGetId_(GxImage* self);

void GxImageTextureSetResource_(GxImage* self, void* resource, GxSize* size);

void GxImageRender_(GxImage* self, SDL_Rect* target, double angle, SDL_RendererFlip orientation);

void GxImageRenderTilePalette_(GxImage* self, SDL_Rect* target);

void GxLoadAnimation(const char* id, const char* pathF,
    int start, int end, int interval, double proportion, bool continuous);

bool GxAnimIsContinous_(GxAnimation* self);

Uint32 GxAnimGetInterval_(GxAnimation* self);

Uint32 GxAnimGetQuantity_(GxAnimation* self);

GxImage* GxAnimGetImage_(GxAnimation* self, Uint32 index);

const char* GxAnimGetId_(GxAnimation* self);

void GxSoundSetChunk_(GxSound* self, Mix_Chunk* chunk);

void GxLoadChunk(const char* id, const char* path);

void GxMusicSetMixMusic_(GxMusic* self, Mix_Music* music);

void GxLoadMusic(const char* id, const char* path);

#endif // !GX_MODULE_H
