#include "../Util/Util.h"
#include "../App/App.h"
#include "../Scene/Scene.h"
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "../Containers/Map/Map.h"
#include "../Folder/Folder.h"
#include "../Containers/Array/Array.h"
#include "../Containers/List/List.h"


//... #forward declarations
static inline sMap* createColorMap(void);
static sMap* createFontMap(void);


//struct
enum AType { IMAGE, MUSIC, SOUND };

typedef struct sApp {

    //...
    int status;    
    sSize size;
   
    //...SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

    //... Scenes and Folders
    sScene* snMain;
    sScene* snActive;
    sScene* snRunning;

    //... containers
	sMap* scenes;
    sMap* folders;
    sMap* colors;
    sMap* fonts;

    //... temporary resources
    sArray* temporary;

    //...font
    TTF_Font* lastFont;
    char* lastFontName;
    int lastFontSize;

    //...assets modules
    sList* aToLoad;
    sList* aLoading;
    sList* aLoaded;
    Uint32 counter;
    Uint32 currentFrame;
    SDL_atomic_t atom;
} sApp;

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
static sApp* self = NULL;

//constructor and destructor
sScene* nAppCreate(const sIni* ini) {

    if(self){ return self->snMain; }
    
    self = calloc(1, sizeof(sApp));
   nUtilAssertAlloc(self);
    self->snActive = NULL;
    self->folders = nMapCreate();
    self->scenes = nMapCreate();
    self->colors = createColorMap();
    self->fonts = createFontMap();
    self->temporary = nArrayCreate();
    self->aToLoad = nListCreate();
    self->aLoading = NULL;
    self->aLoaded = NULL;
    self->currentFrame = 0;
    self->lastFont = NULL;
    self->lastFontName = NULL;
    self->lastFontSize = 0;

    SDL_AtomicSet(&self->atom, nUtil_STATUS_NONE);
   
	  //init SDL modules
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
       nAppRuntimeError(SDL_GetError());
    }

    if (TTF_Init() != 0) {
         nAppRuntimeError(SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
        nAppRuntimeError(SDL_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        nAppRuntimeError(SDL_GetError());
    }

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    Uint32 bigger = mode.w > mode.h ? mode.w : mode.h;
    Uint32 smaller = mode.w > mode.h ? mode.h : mode.w;

   nUtilAssertArgument(ini->window);
    sArray* wparams = nUtilSplit(ini->window, "|");
   nUtilAssertArgument(nArraySize(wparams) == 2);

    if(strstr(ini->window, "Landscape")){
	    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        self->size.h = atoi(nArrayAt(wparams, 1));
       nUtilAssertArgument(self->size.h);
        self->size.w = ((double) bigger * self->size.h) / smaller;       
    }
    else if (strstr(ini->window, "Portrait")) {
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
        self->size.w = atoi(nArrayAt(wparams, 1));
       nUtilAssertArgument(self->size.w);
        self->size.h = ((double) bigger * self->size.w) / smaller;       
    }
    nArrayDestroy(wparams);

    const char* title = ini->title ? ini->title : "Gx";
    Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

    if (strcmp(SDL_GetPlatform(), "Windows") == 0) {
        flags = SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE;
    }
      
    if (!(self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, self->size.w, self->size.h, flags))) {
        nAppRuntimeError(SDL_GetError());
    }

    if (!(self->renderer = SDL_CreateRenderer(self->window, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED))) {
         nAppRuntimeError(SDL_GetError());
    }

    //adjust logical size to real output size of maximized window
    if (strcmp(SDL_GetPlatform(), "Windows") == 0) {       
        sSize rsize = {0, 0};      
        SDL_GetRendererOutputSize(self->renderer, &rsize.w, &rsize.h);       
        if(strstr(ini->window, "Landscape")){
	        self->size.w = (int) (((double)rsize.w * self->size.h) / rsize.h);
        }
        else if (strstr(ini->window, "Portrait")) {
            self->size.h = (int)(((double) rsize.h * self->size.w)/ rsize.w);
        }        
        SDL_SetWindowResizable(self->window, false);
    }
    
    //present window
    SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
    SDL_RenderClear(self->renderer);
    SDL_RenderPresent(self->renderer);  
   
    SDL_SetRenderDrawBlendMode(self->renderer, SDL_BLENDMODE_BLEND);

    self->status = nUtil_STATUS_NONE;
    self->snMain = nSceneCreate(ini);
    return self->snMain;
}

sRect* nAppCalcDest(sRect* src, sRect* dest) {
#define intround(x) ((x) >= 0.0 ? (int) ((x) + 0.5) : (int) ((x) - 0.5))
    sSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nAppSDLRenderer(), &wsize.w, &wsize.h);
    if (wsize.w == self->size.w) {
        *dest = *src;            
    }
    else {
        dest->x = intround(((double)src->x * wsize.w) / self->size.w);       
        dest->y = intround(((double)src->y * wsize.h) / self->size.h);        
        int x1 = intround(((double)(src->x + src->w) * wsize.w) / self->size.w); 
        int y1 = intround(((double)(src->y + src->h) * wsize.h) / self->size.h);
        dest->w = x1 - dest->x;
        dest->h = y1 - dest->y;
    }
    return dest;
#undef intround
}

sRect* nAppCalcLabelDest(sRect* src, sRect* dest) {
    nAppCalcDest(src, dest);
    SDL_Point center = {dest->x + dest->w/2, dest->y + dest->h/2};
    dest->x = center.x - src->w/2;
    dest->y = center.y - src->h/2;
    dest->w = src->w;
    dest->h = src->h;
    return dest;
}

bool nAppIsCreated() {
    return self;
}

static void destroy() {
    if (self) {
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        nListDestroy(self->aToLoad);
        nListDestroy(self->aLoading);
        nListDestroy(self->aLoaded);
        nMapDestroy(self->scenes);
        nSceneDestroy_(self->snMain);
        nArrayDestroy(self->temporary);
        nMapDestroy(self->folders);
        nMapDestroy(self->colors);
        nMapDestroy(self->fonts);
        if (self->lastFont) {
            TTF_CloseFont(self->lastFont);
        }
        if (self->lastFontName) {
            free(self->lastFontName);
        }
        free(self);
        self = NULL;
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
    }
}

//acessors and mutators
SDL_Window* nAppSDLWindow(void) {
    return self->window;
}

SDL_Renderer* nAppSDLRenderer(void) {
    return self->renderer;
}

sSize nAppLogicalSize(void) {
    return self->size;
}

bool nAppIsRunning() {
    return self->status == nUtil_STATUS_RUNNING;
}

void nAppAddScene_(sScene* scene) {
    if(self->snMain != NULL){
        nMapSet(self->scenes, nSceneName(scene), scene, nSceneDestroy_);
    }
}

sScene* nAppGetScene(const char* id) {
    return nMapGet(self->scenes, id);
}

void nAppAddFolder_(sFolder* folder) {   
   nMapSet(self->folders, nFolderName(folder), folder, nFolderDestroy_);
}

sFolder* nAppGetFolder(const char* id) {
    return nMapGet(self->folders, id);
}

void nAppLoadSDLSurface_(sImage* image, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
   nUtilAssertAlloc(asset);
    *asset = (Asset) {
        .type = IMAGE,
        .mod = image,
        .path = nUtilCreateString(path)
    };
    nListPush(self->aToLoad, asset, (sDtor) destroyAsset);
}

void nAppLoadMixChunk_(sChunk* sound, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
   nUtilAssertAlloc(asset);
    *asset = (Asset) {
        .type = SOUND,
        .mod = sound,
        .path = nUtilCreateString(path)
    };
    nListPush(self->aToLoad, asset, (sDtor)  destroyAsset);
}

void nAppLoadMixMusic_(sMusic* music, const char* path) {
     Asset* asset = malloc(sizeof(Asset));
    nUtilAssertAlloc(asset);
    *asset = (Asset) {
        .type = MUSIC,
        .mod = music,
        .path = nUtilCreateString(path)
    };
    nListPush(self->aToLoad, asset, (sDtor) destroyAsset);
}


static void mainLoadAsset(Asset* asset) {
    if (asset) {
        switch (asset->type) {
            case IMAGE: {
                SDL_Surface* surface = asset->resource;
                sSize size = { surface->w,  surface->h };
                SDL_Texture* texture = SDL_CreateTextureFromSurface(
                    self->renderer, surface
                );
                SDL_FreeSurface(surface);
                if (!texture) nAppRuntimeError(SDL_GetError());
                nFolderSetSDLTexture_(asset->mod, texture, &size);
                break;
            }
            case SOUND: {
                nFolderSetMixChunk_(asset->mod, asset->resource);
                break;
            }
            case MUSIC: {
                nFolderSetMixMusic_(asset->mod, asset->resource);
                break;
            }
        }
    }
}

static int threadLoadAsset() {

    for(Asset* asset = nListBegin(self->aLoading); asset != NULL;
        asset = nListNext(self->aLoading)){

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
        if (!asset->resource) nAppRuntimeError(SDL_GetError());
    }

    SDL_AtomicSet(&self->atom, nUtil_STATUS_LOADED);
    return 0;
}


void nAppRun() {
    nUtilAssertState(self);
    self->counter = SDL_GetTicks();
    
    //run
    self->status = nUtil_STATUS_RUNNING;
    nAppLoadScene(self->snMain);

    while (self->status == nUtil_STATUS_RUNNING) {

        bool activeIsReady =  self->snActive ?
            nSceneStatus(self->snActive) == nUtil_STATUS_RUNNING : false;

        self->counter = SDL_GetTicks();
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) nSceneOnLoopBegin_(self->snActive);

        self->snRunning = self->snMain;
        nSceneOnLoopBegin_(self->snMain);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT){
                self->status = nUtil_STATUS_UNLOADING;
            }
            else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
                    SDL_SetWindowResizable(self->window, false);
                }
                else if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                    SDL_SetWindowResizable(self->window, true);
                }
            }
           
            self->snRunning = self->snActive;
            if (activeIsReady && self->snActive) nSceneOnSDLEvent_(self->snActive, &e);

            self->snRunning = self->snMain;
            nSceneOnSDLEvent_(self->snMain, &e);
        }

        //... update
        self->snRunning = self->snActive;
        if (self->snActive) nSceneUpdate_(self->snActive);

        self->snRunning = self->snMain;
        nSceneUpdate_(self->snMain);


        //...load assets
        if (self->aLoaded == NULL && 
            SDL_AtomicGet(&self->atom) == nUtil_STATUS_LOADED) 
        {
            self->aLoaded = self->aLoading;
            self->aLoading = NULL;
            SDL_AtomicSet(&self->atom, nUtil_STATUS_NONE);
        }

        if (nListSize(self->aToLoad) && 
            SDL_AtomicGet(&self->atom) == nUtil_STATUS_NONE) 
        {
            self->aLoading = self->aToLoad;
            self->aToLoad = nListCreate();
            SDL_AtomicSet(&self->atom, nUtil_STATUS_LOADING);
            SDL_Thread* thread = (
                SDL_CreateThread((SDL_ThreadFunction) threadLoadAsset, "loaderThread", self)
            );
            SDL_DetachThread(thread);
        }

        if (self->aLoaded) {
            do {
                Asset* asset = nListFirst(self->aLoaded);
                mainLoadAsset(asset);
                nListRemove(self->aLoaded, asset);
            } while(nListSize(self->aLoaded) && (SDL_GetTicks() - self->counter <= 12));

            if(nListSize(self->aLoaded) == 0){
                nListDestroy(self->aLoaded);
                self->aLoaded = NULL;
            }
        }
        
        {
        /* 
            Correct android screen initialization: without this block, in Android
            the first screen shows strange colored images.
            It's not clear the origin of the bug, if it occurs just in Android or if it occurs just in my old cellphone.
        */
            static int firstScreen = 0;
            if(firstScreen < 30){
                SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
                SDL_RenderClear(self->renderer);
                firstScreen++;
            }
        }

        //... present
        SDL_RenderPresent(self->renderer);
       
        self->currentFrame++;

        //call loop end handlers
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) nSceneOnLoopEnd_(self->snActive);

        self->snRunning = self->snMain;
        nSceneOnLoopEnd_(self->snMain);
        nArrayClean(self->temporary);

         //clear window
        SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
        SDL_RenderClear(self->renderer);
    }
    destroy();
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
}

void nAppLoadScene(sScene* scene) {

    if (scene == self->snActive) {
        return;
    }

    self->snRunning = scene;
    nScenePreLoad_(scene);

    if (scene != self->snMain){
        if(self->snActive){
            self->snRunning = self->snActive;
            nSceneUnLoad_(self->snActive);
        }
        self->snActive = scene;
    }
}

sScene* nAppGetRunningScene() {
    return self->snRunning;
}

sScene* nAppGetMainScene(void) {
    return self->snMain;
}


void nAppCountCallsPerFrame(Uint32* counter, Uint32* frame, const char* callId) {  
    
    if (*frame != self->currentFrame) {        
        printf("%s called %u times\n", callId, *counter);
        *frame = self->currentFrame;
        *counter = 0;
    }
    *counter = *counter + 1;
}


void nAppAlert(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", message, NULL);
}

void nAppRuntimeError(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", message, NULL);
#ifndef NDEBUG
    SDL_TriggerBreakpoint();
#endif
	exit(EXIT_FAILURE);
}

void nAppPlayMusic(const char* path, int loops) {
    Mix_Music* music = nFolderGetMixMusic(path);
    Mix_PlayMusic(music, loops);
 }

void nAppPlayChunk(const char* path, int loops) {
    Mix_Chunk* chunk = nFolderGetMixChunk(path);
    Mix_PlayChannel(-1, chunk, loops);
 }

int nAppStopMusic(void){
    return Mix_HaltMusic();
}

void nAppPauseMusic(void){
    Mix_PauseMusic();
}

void nAppResumeMusic(void) {
    Mix_ResumeMusic();
}

int  nAppIsPlayingMusic(void) {
    return Mix_PlayingMusic();
}

static inline void createColor(sMap* map, const char* name, SDL_Color* color) {
    SDL_Color* ncolor = malloc(sizeof(SDL_Color));
   nUtilAssertAlloc(ncolor);
    *ncolor = *color;
    nMapSet(map, name, ncolor, free);
}

static sMap* createColorMap(void) {

    sMap* map = nMapCreate();

    createColor(map, "Black", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "White", &(SDL_Color){255, 255, 255, 255});
    createColor(map, "Gainsboro", &(SDL_Color){220, 220, 220, 255});
    createColor(map, "Blue", &(SDL_Color){0, 0, 255, 255});
    createColor(map, "Aqua", &(SDL_Color){0, 255, 255, 255});
    createColor(map, "Coral", &(SDL_Color){255, 127, 80, 255});
    createColor(map, "Crimson", &(SDL_Color){220, 20, 60, 255});   
    createColor(map, "FireBrick", &(SDL_Color){178, 34, 34, 255});    
    createColor(map, "Gold", &(SDL_Color){255, 215, 0, 255});
    createColor(map, "Green", &(SDL_Color){0, 128, 0, 255});   
    createColor(map, "Red", &(SDL_Color){255, 0, 0, 255});
    createColor(map, "Salmon", &(SDL_Color){250, 128, 114, 255});  
    createColor(map, "Teal", &(SDL_Color){0, 128, 128, 255});
    createColor(map, "Tomato", &(SDL_Color){255, 99, 71, 255});
    createColor(map, "Yellow", &(SDL_Color){0, 0, 0, 255});
    return map;
}

SDL_Color* nAppConvertColor(SDL_Color* destination, const char* color) {

    SDL_Color* mapcolor = NULL;
    char* clone = nUtilCloneString(color, (char[32]){0}, 32);
    clone = nUtilTrim(clone, (char[32]){'\0'}, 32);
    int len = (int) strlen(clone);

	if(clone[0] == '(' && clone[len - 1] == ')'){

		//remove parenthesis
		(clone++)[len - 1] = '\0';

		//get tokens
        sArray* tokens = nAppTokenize(clone, ",");
       nUtilAssertArgument(nArraySize(tokens) == 4);

        char* r = nArrayAt(tokens, 0);
        char* g = nArrayAt(tokens, 1);
        char* b = nArrayAt(tokens, 2);
        char* a = nArrayAt(tokens, 3);

		//assign color components
		destination->r = (Uint8) atoi(r);
		destination->g = (Uint8) atoi(g);
		destination->b = (Uint8) atoi(b);
		destination->a = (Uint8) atoi(a);
	}
	else if((mapcolor = nMapGet(self->colors, color))){
        *destination = *mapcolor;
	}
    else {
       nUtilAssertArgument(false);
    }
    return destination;
}


static sMap* createFontMap(void) {
#define FPATH "Gx/Font/FiraSans/FiraSans-"

    sMap* fonts = nMapCreate();
    nMapSet(fonts, "Default", nUtilCreateString(FPATH "Regular.ttf"), free);
    nMapSet(fonts, "Italic", nUtilCreateString(FPATH "Italic.ttf"), free);
    nMapSet(fonts, "Bold", nUtilCreateString(FPATH "Bold.ttf"), free);
    nMapSet(fonts, "BoldItalic", nUtilCreateString(FPATH "BoldItalic.ttf"), free);
    return fonts;

#undef FPATH
}

void nAppAddFont(const char* name, const char* path){
  nUtilAssertNullPointer(name);
  nUtilAssertNullPointer(path);
    TTF_Font* teste = TTF_OpenFont(path, 16);
    if (!teste) {
        nAppRuntimeError(sf("Could not open path %s", path));
    }
    TTF_CloseFont(teste);
    nMapSet(self->fonts, name, nUtilCreateString(path), free);    
}

const char* nAppGetFontPath_(const char* name) {
    return nMapGet(self->fonts, name);
}

TTF_Font* nAppLoadFont_(const char* name, int size) {
    nUtilAssertState(name);

    if (self->lastFont) {
        if (strcmp(name, self->lastFontName) == 0 && self->lastFontSize == size) {
            return self->lastFont;
        }
        else {
            TTF_CloseFont(self->lastFont);
            self->lastFont = NULL;
            free(self->lastFontName);
            self->lastFontName = NULL;
        }
    }

    const char* path = nMapGet(self->fonts, name);
    nUtilAssertResourceNotFound(path);
    self->lastFont = TTF_OpenFont(path, size);
    if(!self->lastFont){
        nAppRuntimeError(TTF_GetError());
    }
    self->lastFontName = nUtilCreateString(name);
    self->lastFontSize = size;
    return self->lastFont;
}

char* sf(const char* format, ...) {   
    static char buffer[1024]; //1kb
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
    char* value = nUtilCreateString(buffer);
    nArrayPush(self->temporary, value, free);
	return value;
}

sArray* nAppTokenize(const char* str, const char* sep){
    sArray* response = nUtilSplit(str, sep);
    nArrayPush(self->temporary, response, nArrayDestroy);
    return response;
}
