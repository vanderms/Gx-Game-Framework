#ifndef GX_APP_H
#define GX_APP_H
#include "../Utilities/Util.h"
#include <SDL_ttf.h>


typedef struct sAppNamespace {
	GxScene* (*create)(const sIni* ini);	
	void (*run)(void);
	bool (*isCreated)(void);
	bool (*isRunning)(void);	
	SDL_Window* (*SDLWindow)(void);
	SDL_Renderer* (*SDLRenderer)(void);	
	GxScene* (*getScene)(const char* id);
	GxSize (*logicalSize)(void);
	void (*loadScene)(GxScene* scene);
	void (*addFont)(const char* name, const char* path);
	GxScene* (*getRunningScene)(void);
	GxScene* (*getMainScene)(void);
	void (*alert)(const char* message);
	void (*runtimeError)(const char* message);
	void (*playMusic)(const char* path, int loops);
	void (*playChunk)(const char* path, int loops);
	int (*stopMusic)(void);
	void (*pauseMusic)(void);
	void (*resumeMusic)(void);
	int (*isPlayingMusic)(void);
	void (*convertColor)(SDL_Color* dest, const char* color);	
	sArray* (*tokenize)(const char* str, const char* sep);	
	char* (*sf)(const char* format, ...);
	SDL_Rect* (*calcDest)(SDL_Rect* src, SDL_Rect* dest);
	SDL_Rect* (*calcLabelDest)(SDL_Rect* src, SDL_Rect* dest);
	struct sAppPrivate {
		void (*addScene)(GxScene* scene);
		void (*addFolder)(GxFolder* handler);
		GxFolder* (*getFolder)(const char* id);
		const char*(*getFontPath)(const char* name);
		void (*loadSDLSurface)(GxImage* image, const char* path);
		void(*loadMixChunk)(GxSound* sound, const char* path);
		void (*loadMixMusic)(GxMusic* music, const char* path);
	}* prv;
} sAppNamespace;

extern const sAppNamespace* nApp;

#endif // !GX_APP_H

