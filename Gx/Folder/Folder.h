#ifndef GX_MODULE_H
#define GX_MODULE_H
#include "../Utilities/Util.h"


extern const struct sFolderNamespace {

    void (*create)(const char* id, void(*loader)(void));

    Mix_Music* (*getMixMusic)(const char* path);

    Mix_Chunk* (*getMixChunk)(const char* path);

    SDL_Texture* (*getSDLTexture)(const char* path);

    void (*loadImage)(const char* id, const char* path, SDL_Rect* src, double proportion);

    void (*loadTileset)(const char* id, const char* pathF, int start, int end, double proportion);

    void (*createTilesetFromImage)(const char* image, sSize size, sMatrix matrix);

    void (*createTilemap)(const char* folderName, const char* name, const char* group, sSize size, sMatrix matrix, int* sequence);

    sSize (*getImageSize)(const char* path);   

    void (*loadAnimation)(const char* id, const char* pathF, int start, int end, int interval, double proportion, bool continuous);

    void (*loadChunk)(const char* id, const char* path);
    
    void (*loadMusic)(const char* id, const char* path);

    const struct sFolderPrivateNamespace {
        
        void (*destroy)(sFolder* self);
        bool(*hasStatus)(sFolder* self, int status);
        char* (*id)(sFolder* self);
        int (*getPercLoaded)(sFolder* self);
        void (*incRefCounter)(sFolder* self);
        void (*decRefCounter)(sFolder* self);
        
        sImage* (*getImage)(sFolder* self, const char* id);
        sAnimation* (*getAnimation)(sFolder* self, const char* id);
       
        void (*setMixChunk)(sChunk* self, Mix_Chunk* chunk);
        void (*setMixMusic)(sMusic* self, Mix_Music* music);
        void (*setSDLTexture)(sImage* self, void* resource, sSize* size);              
    
        void (*destroyImage)(sImage* self);
        const char* (*getImageId)(sImage* self);            
        sSize (*getImageSize)(sImage* self);
        sImage* (*createText)(const char* text, const char* font, int size, SDL_Color* color);
        void (*renderImage)(sImage* self, SDL_Rect* target, double angle, SDL_RendererFlip orientation, Uint8 opacity);
        void (*renderTilePallete)(sImage* self, SDL_Rect* target, Uint8 opacity);

        const char* (*getAnimId)(sAnimation* self);  
        bool (*isAnimContinuous)(sAnimation* self);
        Uint32 (*getAnimInterval)(sAnimation* self);
        Uint32 (*getAnimQuantity)(sAnimation* self);
        sImage* (*getAnimImage)(sAnimation* self, Uint32 index);

    }* p;
}* nFolder;

#endif // !GX_MODULE_H
