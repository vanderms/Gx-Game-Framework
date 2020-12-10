#ifndef GX_MODULE_H
#define GX_MODULE_H
#include "../Util/Util.h"


void nFolderCreate(const char* name, void(*loader)(void));

bool nFolderHasStatus(sFolder* self, int status);

char* nFolderName(sFolder* self);

int nFolderGetPercLoaded(sFolder* self);

Mix_Music* nFolderGetMixMusic(const char* path);

Mix_Chunk* nFolderGetMixChunk(const char* path);

sImage* nFolderGetImage(sFolder* self, const char* id);

sAnimation* nFolderGetAnimation(sFolder* self, const char* id);

void nFolderLoadImage(const char* id, const char* path, sRect* src, double proportion);

void nFolderLoadTileset(const char* id, const char* pathF, int start, int end, double proportion);

void nFolderCreateTilesetFromImage(const char* image, sSize size, sMatrix matrix);

void nFolderLoadAnimation(const char* id, const char* pathF, int start, int end, int interval, double proportion, bool continuous);

void nFolderLoadChunk(const char* id, const char* path);

void nFolderLoadMusic(const char* id, const char* path);

void nFolderDestroy_(sFolder* self);

void nFolderIncRefCounter_(sFolder* self);
void nFolderDecRefCounter_(sFolder* self);

void nFolderSetMixChunk_(sChunk* self, Mix_Chunk* chunk);
void nFolderSetMixMusic_(sMusic* self, Mix_Music* music);
void nFolderSetSDLTexture_(sImage* self, void* resource, sSize* size);      

   


const char* nImageName(sImage* self);
sFolder* nImageFolder(sImage* self);
const sSize* nImageSize(sImage* self);
SDL_Texture* nImageSDLTexture(sImage* self);
const sRect* nImageSrc(sImage* self);
double nImageProportion(sImage* self);
sImage* nImageCreateText(
    const char* text, const char* font, int size, SDL_Color* color
);
void nImageRender(sImage* self, 
    sRect* target, double angle, SDL_RendererFlip orientation, Uint8 opacity
);
void nImageDestroyText(sImage* self);

const char* nAnimName(sAnimation* self);

Uint32 nAnimInterval(sAnimation* self);

Uint32 nAnimQuantity(sAnimation* self);

bool nAnimIsContinuous(sAnimation* self);

sImage* nAnimGetImage(sAnimation* self, Uint32 index);




#endif // !GX_MODULE_H
