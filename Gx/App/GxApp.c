#include "../Utilities/GxUtil.h"
#include "../App/GxApp.h"
#include "../Ini/GxIni.h"
#include "../Scene/GxScene.h"
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "../Map/GxMap.h"
#include "../Folder/GxFolder.h"
#include "../Array/GxArray.h"
#include "../List/GxList.h"

//... #forward declarations
static inline GxMap* createColorMap(void);
static GxMap* createFontMap(void);


//struct
enum AType { IMAGE, MUSIC, SOUND };

typedef struct GxApp {

    //...
    int status;
    GxSize size;

    //...SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

    //... Scenes and Folders
    GxScene* snMain;
    GxScene* snActive;
    GxScene* snRunning;

    //... containers
	GxMap* scenes;
    GxMap* folders;
    GxMap* colors;
    GxMap* fonts;

    //... temporary resources
    GxArray* temporary;

    //...assets modules
    GxList* aToLoad;
    GxList* aLoading;
    GxList* aLoaded;
    uint32_t counter;
    SDL_atomic_t atom;
} GxApp;

typedef struct Asset {
    enum AType type;
    void* mod;
    void* resource;
    char* path;
} Asset;

static inline void destroyAsset(Asset* self) {
    if (self) {
        free(self->path);
        free(self);
    }
}

//static instance
static GxApp* self = NULL;

//constructor and destructor
void GxCreateApp(const GxIni* ini) {

    if(self){ return; }
    self = calloc(1, sizeof(GxApp));
    GxAssertAllocationFailure(self);
    self->snActive = NULL;
    self->folders = GmCreateMap();
    self->scenes = GmCreateMap();
    self->colors = createColorMap();
    self->fonts = createFontMap();
    self->temporary = GxCreateArray();
    self->aToLoad = GxCreateList();
    self->aLoading = NULL;
    self->aLoaded = NULL;

    SDL_AtomicSet(&self->atom, GxStatusNone);


    //set hint
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

	  //init SDL modules
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
       GxFatalError(SDL_GetError());
    }

    if (TTF_Init() != 0) {
         GxFatalError(SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
        GxFatalError(SDL_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        GxFatalError(SDL_GetError());
    }

    //calc window size
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    self->size.w = mode.w > mode.h ? mode.w : mode.h;
    self->size.h = mode.w > mode.h ? mode.h : mode.w;
    self->size.w = (int) ((self->size.w * 360) / (double) self->size.h);
    self->size.h = 360;

    const char* title = ini->title ? ini->title : "Gx";
    Uint32 flags = GxAndroid ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN;

    //init window and renderer
    if (!(self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, self->size.w, self->size.h, flags))) {
        GxFatalError(SDL_GetError());
    }

    if (!(self->renderer = SDL_CreateRenderer(self->window, -1, SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC))) {
         GxFatalError(SDL_GetError());
    }

    //present window
    SDL_SetRenderDrawColor(self->renderer, 255, 255, 255, 255);
    SDL_RenderClear(self->renderer);
    SDL_RenderPresent(self->renderer);

    //in android go fullscreen
    if (strcmp(SDL_GetPlatform(), "Android") == 0) {
        SDL_SetWindowFullscreen(self->window, SDL_WINDOW_FULLSCREEN);
    }

    //set logical size
    SDL_RenderSetLogicalSize(self->renderer, self->size.w, self->size.h);

    self->status = GxStatusNone;
    self->snMain = GxCreateScene(ini);
    GxLoadScene(self->snMain);
    GxRunLoop_();
}

bool GxAppIsCreated_() {
    return self;
}

static void destroyApp_() {
    if (self) {
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        GxDestroyList(self->aToLoad);
        GxDestroyList(self->aLoading);
        GxDestroyList(self->aLoaded);
        GxDestroyMap(self->scenes);
        GxDestroyScene_(self->snMain);
        GxDestroyArray(self->temporary);
        GxDestroyMap(self->folders);
        GxDestroyMap(self->colors);
        GxDestroyMap(self->fonts);
        free(self);
        self = NULL;
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
    }
}

//acessors and mutators
SDL_Window* GxGetSDLWindow() {
    return self->window;
}

SDL_Renderer* GxGetSDLRenderer() {
    return self->renderer;
}

GxSize GxGetWindowSize() {
    return self->size;
}

bool GxAppIsRunning_() {
    return self->status == GxStatusRunning;
}

void GxAddScene_(GxScene* scene) {
    if(self->snMain != NULL){
        GxMapSet(self->scenes, GxSceneGetName(scene), scene, (GxDestructor) GxDestroyScene_);
    }
}

GxScene* GxGetScene(const char* id) {
    return GxMapGet(self->scenes, id);
}

void GxAddFolder_(GxFolder* handler) {
   void GxDestroyFolder_(GxFolder* self);
   GxMapSet(self->folders, GxFolderGetId_(handler), handler, (GxDestructor) GxDestroyFolder_);
}

GxFolder* GxGetFolder_(const char* id) {
    return GxMapGet(self->folders, id);
}

void GxPushTextureToLoad_(GxImage* image, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    GxAssertAllocationFailure(asset);
    *asset = (Asset) {
        .type = IMAGE,
        .mod = image,
        .path = GmCreateString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor) destroyAsset);
}

void GxPushChunkToLoad_(GxSound* sound, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    GxAssertAllocationFailure(asset);
    *asset = (Asset) {
        .type = SOUND,
        .mod = sound,
        .path = GmCreateString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor)  destroyAsset);
}

void gxPushMusicToLoad_(GxMusic* music, const char* path) {
     Asset* asset = malloc(sizeof(Asset));
     GxAssertAllocationFailure(asset);
    *asset = (Asset) {
        .type = MUSIC,
        .mod = music,
        .path = GmCreateString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor) destroyAsset);
}


static inline void mainLoadAsset(Asset* asset) {
    if (asset) {
        switch (asset->type) {
            case IMAGE: {
                SDL_Surface* surface = asset->resource;
                GxSize size = { surface->w,  surface->h };
                SDL_Texture* texture = SDL_CreateTextureFromSurface(
                    self->renderer, surface
                );
                SDL_FreeSurface(surface);
                if (!texture) GxFatalError(SDL_GetError());
                GxImageTextureSetResource_(asset->mod, texture, &size);
                break;
            }
            case SOUND: {
                GxSoundSetChunk_(asset->mod, asset->resource);
                break;
            }
            case MUSIC: {
                GxMusicSetMixMusic_(asset->mod, asset->resource);
                break;
            }
        }
    }
}

static inline int threadLoadAsset() {

    for(Asset* asset = GxListBegin(self->aLoading); asset != NULL;
        asset = GxListNext(self->aLoading)){

        if (asset) {
            switch (asset->type) {
                case IMAGE: {
                    asset->resource = IMG_Load(asset->path);
                    break;
                }
                case SOUND: {
                    asset->resource = Mix_LoadWAV(asset->path);
                    break;
                }
                case MUSIC: {
                    asset->resource = Mix_LoadMUS(asset->path);
                    break;
                }
            }
        }
        if (!asset->resource) GxFatalError(SDL_GetError());
    }


    SDL_AtomicSet(&self->atom, GxStatusLoaded);
    return 0;
}


void GxRunLoop_() {

    self->counter = SDL_GetTicks();

    //run
    self->status = GxStatusRunning;

    while (self->status == GxStatusRunning) {


        bool activeIsReady =  self->snActive ?
            GxSceneGetStatus(self->snActive) == GxStatusRunning : false;

        self->counter = SDL_GetTicks();
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) GxSceneOnLoopBegin_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnLoopBegin_(self->snMain);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) self->status = GxStatusUnloading;

            self->snRunning = self->snActive;
            if (activeIsReady && self->snActive) GxSceneOnSDLEvent_(self->snActive, &e);

            self->snRunning = self->snMain;
            GxSceneOnSDLEvent_(self->snMain, &e);
        }

        //... update
        self->snRunning = self->snActive;
        if (self->snActive) GxSceneOnUpdate_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnUpdate_(self->snMain);


        //...load assets
        if (self->aLoaded == NULL && SDL_AtomicGet(&self->atom) == GxStatusLoaded) {
            self->aLoaded = self->aLoading;
            self->aLoading = NULL;
            SDL_AtomicSet(&self->atom, GxStatusNone);
        }

        if (GxListSize(self->aToLoad) && SDL_AtomicGet(&self->atom) == GxStatusNone) {
            self->aLoading = self->aToLoad;
            self->aToLoad = GxCreateList();
            SDL_AtomicSet(&self->atom, GxStatusLoading);
            SDL_Thread* thread = SDL_CreateThread((SDL_ThreadFunction) threadLoadAsset, "loaderThread", self);
            SDL_DetachThread(thread);
        }

        if (self->aLoaded) {
            do {
                Asset* asset = GxListFirst(self->aLoaded);
                mainLoadAsset(asset);
                GxListRemove(self->aLoaded, asset);
            } while(GxListSize(self->aLoaded) && (SDL_GetTicks() - self->counter <= 12));

            if(GxListSize(self->aLoaded) == 0){
                GxDestroyList(self->aLoaded);
                self->aLoaded = NULL;
            }
        }

        //... present
        SDL_RenderPresent(self->renderer);

        //call loop end handlers
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) GxSceneOnLoopEnd_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnLoopEnd_(self->snMain);
        GxArrayClean(self->temporary);

        //clear window
        SDL_SetRenderDrawColor(self->renderer, 255, 255, 255, 255);
        SDL_RenderClear(self->renderer);
    }
    destroyApp_();
}

void GxLoadScene(GxScene* scene) {

    if (scene == self->snActive) {
        return;
    }

    self->snRunning = scene;
    GxScenePreLoad_(scene);

    if (scene != self->snMain){
        if(self->snActive){
            self->snRunning = self->snActive;
            GxSceneUnload_(self->snActive);
        }
        self->snActive = scene;
    }
}

GxScene* GxGetRunningScene() {
    return self->snRunning;
}

GxScene* GxGetMainScene(void) {
    return self->snMain;
}


void GxAlert(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", message, NULL);
}

 void GxFatalError(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message, NULL);
	exit(EXIT_FAILURE);
}

 void GxPlayMusic(const char* path, int loops) {
    Mix_Music* music = GxFolderGetMusic(path);
    Mix_PlayMusic(music, loops);
 }

 void GxPlayChunk(const char* path, int loops) {
    Mix_Chunk* chunk = GxFolderGetChunk(path);
    Mix_PlayChannel(-1, chunk, loops);
 }

static inline void createColor(GxMap* map, const char* name, SDL_Color* color) {
    SDL_Color* ncolor = malloc(sizeof(SDL_Color));
    GxAssertAllocationFailure(ncolor);
    *ncolor = *color;
    GxMapSet(map, name, ncolor, free);
}

static inline GxMap* createColorMap() {

    GxMap* map = GmCreateMap();

    createColor(map, "Black", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "White", &(SDL_Color){255, 255, 255, 255});
    createColor(map, "Blue", &(SDL_Color){0, 0, 255, 255});
    createColor(map, "Aqua", &(SDL_Color){0, 255, 255, 255});
    createColor(map, "Coral", &(SDL_Color){255, 127, 80, 255});
    createColor(map, "Crimson", &(SDL_Color){220, 20, 60, 255});
    createColor(map, "DarkBlue", &(SDL_Color){0, 0, 139, 255});
    createColor(map, "DarkGreen", &(SDL_Color){0, 100, 0, 255});
    createColor(map, "DimGrey", &(SDL_Color){105, 105, 105, 255});
    createColor(map, "FireBrick", &(SDL_Color){178, 34, 34, 255});
    createColor(map, "ForestGreen", &(SDL_Color){34, 139, 34, 255});
    createColor(map, "Gold", &(SDL_Color){255, 215, 0, 255});
    createColor(map, "Green", &(SDL_Color){0, 128, 0, 255});
    createColor(map, "IndianRed", &(SDL_Color){205, 92, 92, 255});
    createColor(map, "LimeGreen", &(SDL_Color){50, 205, 50, 255});
    createColor(map, "OrangeRed", &(SDL_Color){255, 69, 0, 255});
    createColor(map, "Red", &(SDL_Color){255, 0, 0, 255});
    createColor(map, "Salmon", &(SDL_Color){250, 128, 114, 255});
    createColor(map, "SeaGreen", &(SDL_Color){46, 139, 87, 255});
    createColor(map, "SteelBlue", &(SDL_Color){70, 130, 180, 255});
    createColor(map, "Teal", &(SDL_Color){0, 128, 128, 255});
    createColor(map, "Tomato", &(SDL_Color){255, 99, 71, 255});
    createColor(map, "Yellow", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "YellowGreen", &(SDL_Color){154, 205, 50, 255});

    return map;
}

void GxConvertColor(SDL_Color* destination, const char* color) {

    SDL_Color* mapcolor = NULL;
    char* clone = GxCloneString(color, (char[32]){0}, 32);
    clone = GxTrim(clone, (char[32]){'\0'}, 32);
    int len = (int) strlen(clone);

	if(clone[0] == '(' && clone[len - 1] == ')'){

		//remove parenthesis
		(clone++)[len - 1] = '\0';

		//get tokens
        GxArray* tokens = GxTokenize(clone, ",");
        GxAssertInvalidArgument(GxArraySize(tokens) == 4);

        char* r = GxArrayAt(tokens, 0);
        char* g = GxArrayAt(tokens, 1);
        char* b = GxArrayAt(tokens, 2);
        char* a = GxArrayAt(tokens, 3);

		//assign color components
		destination->r = (Uint8) atoi(r);
		destination->g = (Uint8) atoi(g);
		destination->b = (Uint8) atoi(b);
		destination->a = (Uint8) atoi(a);
	}
	else if((mapcolor = GxMapGet(self->colors, color))){
        *destination = *mapcolor;
	}
    else {
        GxAssertInvalidArgument(false);
    }
}


static GxMap* createFontMap(void) {
    GxMap* fonts = GmCreateMap();
#define FPATH "Assets/PTSerif/PTSerif-"

    GxMapSet(fonts, "Default", GmCreateString(FPATH "Regular.ttf"), free);
    GxMapSet(fonts, "Italic", GmCreateString(FPATH "Italic.ttf"), free);
    GxMapSet(fonts, "Bold", GmCreateString(FPATH "Bold.ttf"), free);
    GxMapSet(fonts, "BoldItalic", GmCreateString(FPATH "BoldItalic.ttf"), free);

#undef FPATH
    return fonts;
}

void GxAddFont(const char* name, const char* path){
    GxAssertNullPointer(name);
    GxAssertNullPointer(path);
    TTF_Font* teste = TTF_OpenFont(path, 16);
    if (!teste) {
        GxFatalError(GxF("Could not open path %s", path));
    }
    TTF_CloseFont(teste);
    GxMapSet(self->fonts, name, GmCreateString(path), free);
}

const char* GxGetFontPath_(const char* name) {
    return GxMapGet(self->fonts, name);
}


//... TEMPORARY RESOURCES FUNCTIONS

char* GxF(const char* format, ...) {
    //... buffer
    static char buffer[1024]; //1kb
    //...
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
    char* value = GmCreateString(buffer);
    GxArrayPush(self->temporary, value, free);
	return value;
}

int* GxI(int value) {
    int* num = malloc(sizeof(int));
    GxAssertAllocationFailure(num);
    *num = value;
    GxArrayPush(self->temporary, num, free);
    return num;
}

double* GxD(double value) {
    double* num = malloc(sizeof(double));
    GxAssertAllocationFailure(num);
    *num = value;
    GxArrayPush(self->temporary, num, free);
    return num;
}

bool* GxB(bool value) {
    bool* boolean = malloc(sizeof(bool));
    GxAssertAllocationFailure(boolean);
    *boolean = value;
    GxArrayPush(self->temporary, boolean, free);
    return boolean;
}

Uint32* GxU(Uint32 value) {
    Uint32* num = malloc(sizeof(Uint32));
    *num = value;
    GxArrayPush(self->temporary, num, free);
    return num;
}

GxArray* GxTokenize(const char* str, const char* sep){
    GxArray* response = GmArraySplit(str, sep);
    GxArrayPush(self->temporary, response, GxDestroyArray);
    return response;
}
