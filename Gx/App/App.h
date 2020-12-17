#ifndef GX_APP_H
#define GX_APP_H
#include "../Util/Util.h"


sScene* nAppCreate(const sIni* ini);	
void nAppRun(void);
bool nAppIsCreated(void);
bool nAppIsRunning(void);	
SDL_Window* nAppSDLWindow(void);
SDL_Renderer* nAppSDLRenderer(void);	
sScene* nAppGetScene(const char* id);
sFolder* nAppGetFolder(const char* id);
sSize nAppLogicalSize(void);
void nAppLoadScene(sScene* scene);
void nAppAddFont(const char* name, const char* path);

sScene* nAppGetRunningScene(void);
sScene* nAppGetMainScene(void);
void nAppAlert(const char* message);
void nAppRuntimeError(const char* message);
void nAppPlayMusic(const char* path, int loops);
void nAppPlayChunk(const char* path, int loops);
int nAppStopMusic(void);
void nAppPauseMusic(void);
void nAppResumeMusic(void);
int  nAppIsPlayingMusic(void);
SDL_Color* nAppConvertColor(SDL_Color* dst, const char* color);	
sArray* nAppTokenize(const char* str, const char* sep);	
char* sf(const char* format, ...);
sRect* nAppCalcDest(sRect* src, sRect* dest);
sRect* nAppCalcLabelDest(sRect* src, sRect* dest);
void nAppCountCallsPerFrame(Uint32* counter, Uint32* frame, const char* callId);
void nAppAddScene_(sScene* scene);
void nAppAddFolder_(sFolder* handler);
const char*nAppGetFontPath_(const char* name);
TTF_Font* nAppLoadFont_(const char* name, int size);
void nAppLoadSDLSurface_(sImage* image, const char* path);
void nAppLoadMixChunk_(sChunk* sound, const char* path);
void nAppLoadMixMusic_(sMusic* music, const char* path);
#endif // !GX_APP_H

