#ifndef GX_H
#define GX_H
#include "../Public/GxPublic.h"
#include "../Ini/GxIni.h"

//... NAMESPACES
struct GxSDLNamespace {
	SDL_Window* (*getWindow)(void);
	SDL_Renderer* (*getRenderer)(void);	
};

struct GxEventNamespace {
	const int LOAD;
	const int LOOP_BEGIN;
	const int UPDATE;
	const int PRE_GRAPHICAL;
	const int PRE_RENDER;
	const int LOOP_END;
	const int UNLOAD;
	const int KEYBOARD;
	const int MOUSE;
	const int FINGER;
	const int SDL_DEFAULT;
	const int PRE_CONTACT;
	const int CONTACT_BEGIN;
	const int CONTACT_END;
	const int TIMEOUT;
	const int DESTROY;
	const int ELEM_REMOVAL;
};

typedef struct GxAppNamespace {
	GxScene* (*create)(const GxIni* ini);	
	void (*run)(void);
	const struct GxSDLNamespace* SDL;
	const struct GxEventNamespace* event;
	GxScene* (*getScene)(const char* id);
	GxSize (*getWindowSize)(void);
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
	void (*convertColor)(SDL_Color* destination, const char* color);	
	GxArray* (*tokenize)(const char* str, const char* sep);
	GxData* (*i)(const int value);
	GxData* (*u)(const Uint32 value);
	GxData* (*f)(const double value);
	GxData* (*b)(const bool value);
	GxData* (*c)(const char value);
	GxData* (*sf)(const char* format, ...);
	GxData* (*ptr)(const void* value);
	GxData* (*list)(void);
	GxData* (*array)(void);
	GxData* (*map)(void);
	GxData* (*rect)(const SDL_Rect* rect);
	GxData* (*vector)(const GxVector* vector);
	GxData* (*point)(const SDL_Point* point);
	GxData* (*size)(const GxSize* size);
	GxData* (*matrix)(const GxMatrix* matrix);	
	void (*freeTarget)(GxEvent* e);
} GxAppNamespace;


typedef struct GxArrayNamespace{
	GxArray* (*create)(void);
	void (*destroy)(GxArray* self);	
	Uint32 (*size)(GxArray* self);
	Uint32 (*capacity)(GxArray* self);
	void* (*at)(GxArray* self, Uint32 index);	
	void (*push)(GxArray* self, void* value, GxDestructor dtor);
	void (*insert)(GxArray* self, Uint32 index, void* value, GxDestructor dtor);
	void (*remove)(GxArray* self, Uint32 index);
	int (*removeByValue)(GxArray* self, void* value);
	int64_t (*indexOf)(GxArray* self, void* value);
	void (*reserve)(GxArray* self, Uint32 capacity);
	void (*clean)(GxArray* self);
	void (*sort)(GxArray* self, GxComp compare);
}GxArrayNamespace;


typedef struct GxButtonNamespace {
	GxElement* (*create)(const GxIni* ini, Uint32 inputs, int keyCode);	
	Uint32 (*getStatus)(GxElement* elem);
	bool (*hasStatus)(GxElement* elem, Uint32 status);
	const Uint32 KEYBOARD;
	const Uint32 FINGER;
	const Uint32 MOUSE;	
	const Uint32 NONE;
	const Uint32 ON;
	const Uint32 HOVER;
	const Uint32 CLICK;
	const Uint32 DOWN;
	const Uint32 UP;
} GxButtonNamespace;


typedef struct GxElemNamespace {
	
	//...Element
	GxElement* (*create)(const GxIni* ini);	
	GxElement* (*createTilemap)(const char* tilePath, const GxIni* ini);	
	void (*remove)(GxElement* self);	
	void* (*getTarget)(GxElement* self);
	Uint32 (*getID)(GxElement* self);
	GxScene* (*getScene)(GxElement* self);

	const SDL_Rect* (*getPos)(GxElement* self);
	void (*setPos)(GxElement* self, SDL_Rect pos);
	GxPoint (*getCenter)(GxElement* self);
	
	const char* (*getClassName)(GxElement* self);
	bool (*hasClass)(GxElement* self, const char* type);
	
	bool (*hasHandler)(GxElement* self, int type);
	GxHandler (*getHandler)(GxElement* self, int type);	
	void (*addRequestHandler)(GxElement* self, const char* request, GxRequestHandler handler);
	
	bool (*isPhysical)(GxElement* self);
	bool (*isRenderable)(GxElement* self);
	bool (*hasDynamicBody)(GxElement* self);
	bool (*hasFixedBody)(GxElement* self);
	bool (*hasRelativePosition)(GxElement* self);
	bool (*hasAbsolutePosition)(GxElement* self);	
	
	void (*setChild)(GxElement* self, void* child);
	void* (*getChild)(GxElement* self);
	
	//...RigidBody
	bool (*isOnGround)(GxElement* self);

	Uint32 (*getCmask)(GxElement* self);
	void (*setCmask)(GxElement* self, Uint32 mask);

	int (*getPreference)(GxElement* self);
	void (*setPreference)(GxElement* self, int value);

	bool (*hasFriction)(GxElement* self);
	void (*setFriction)(GxElement* self, bool value);

	GxVector (*getVelocity)(GxElement* self);
	void (*setVelocity)(GxElement* self, GxVector velocity);

	int (*getVely)(GxElement* self);
	void (*setVely)(GxElement* self, int vel);

	int (*getVelx)(GxElement* self);
	void (*setVelx)(GxElement* self, int vel);

	void (*accelerate)(GxElement* self, double x, double y);

	bool (*isMoving)(GxElement* self);

	double (*getElasticity)(GxElement* self);
	void (*setElasticity)(GxElement* self, double elasticity);

	double (*getRestitution)(GxElement* self);
	void (*setRestitution)(GxElement* self, double restitution);

	int (*getMaxgvel)(GxElement* self);
	void (*setMaxgvel)(GxElement* self, int value);

	GxArray* (*getContacts)(GxElement* self, int types);	

	GxVector (*move)(GxElement* self, GxVector vector, bool force);
	void (*moveTo)(GxElement* self, GxPoint pos, bool force);

	//..Renderable	
	int (*getZIndex)(GxElement* self);
	void (*setZIndex)(GxElement* self, int index);

	Uint8 (*getOpacity)(GxElement* self);
	void (*setOpacity)(GxElement* self, Uint8 value);

	int (*getOrientation)(GxElement* self);
	void (*setOrientation)(GxElement* self, int value);

	const char* (*getImage)(GxElement* self);
	void (*setImage)(GxElement* self, const char* path);

	const char* (*getAnimation)(GxElement* self);
	void (*setAnimation)(GxElement* self, const char* path);

	const char* (*getAlignment)(GxElement* self);
	void (*setAlignment)(GxElement* self, const char* value);

	bool (*isHidden)(GxElement* self);
	void (*hide)(GxElement* self);
	void (*show)(GxElement* self);

	double (*getAngle)(GxElement* self);
	void (*setAngle)(GxElement* self, double angle);

	double (*getProportion)(GxElement* self);
	void (*setProportion)(GxElement* self, double proportion);
	void (*setToFit)(GxElement* self, const char* axis);
		
	const SDL_Color* (*getBackgroundColor)(GxElement* self);
	void (*setBackgroundColor)(GxElement* self, const char* color);

	int (*getBorderSize)(GxElement* self);	
	const SDL_Color* (*getBorderColor)(GxElement* self);
	void (*setBorder)(GxElement* self, const char* border);

	void (*setText)(GxElement* self, const char* text, ...);	
	const char* (*getText)(GxElement* self);
	
	void (*setFontSize)(GxElement* self, int size);
	int (*getFontSize)(GxElement* self);

	void (*setFont)(GxElement* self, const char* font);
	const char* (*getFont)(GxElement* self);

	const SDL_Color* (*getColor)(GxElement* self);
	void (*setColor)(GxElement* self, const char* color);	

	SDL_Rect (*getPositionOnWindow)(GxElement* self);

	GxData* (*send)(Uint32 receiverID, const char* description, GxData* data);
	void (*delegate)(GxElement* self, const char* sceneReq, GxElemID elem, const char* elemReq);

	const int NONE;
	const int ABSOLUTE;
	const int RELATIVE;
	const int FIXED;
	const int DYNAMIC;
	const int FORWARD;
	const int BACKWARD;

} GxElemNamespace;


typedef struct GxFolderNamespace {	
	void (*create)(const char* id, void(*loader)(void));	
	void (*loadImage)(const char* id, const char* path, 
		SDL_Rect* src, double proportion
	);		
	void (*loadTileset)(const char* id, 
		const char* pathF, int start, int end, double proportion
	);		
	void (*createTiles)(const char* image, GxSize size, GxMatrix matrix);		
	void (*loadAnimation)(const char* id, const char* pathF, 
		int start, int end, int interval, double proportion, bool continuous
	);	
	GxSize (*getImageSize)(const char* path);
	void (*loadChunk)(const char* id, const char* path);
	void (*loadMusic)(const char* id, const char* path);
	Mix_Chunk* (*getChunk)(const char* path);
	Mix_Music* (*getMusic)(const char* path);
	SDL_Texture*(*getTexture)(const char* path);
} GxFolderNamespace;


typedef struct GxListNamespace {	
	GxList* (*create)(void);
	void (*destroy)(GxList* self);
	int (*size)(GxList* self);
	void* (*first)(GxList* self);
	void* (*last)(GxList* self);
	void* (*at)(GxList* self, int index);
	bool (*contains)(GxList* self, void* value);
	void* (*begin)(GxList* self);
	void* (*next)(GxList* self);
	void (*push)(GxList* self, void* value, GxDestructor dtor);
	void (*insert)(GxList* self, int index, void* value, GxDestructor dtor);
	bool (*replace)(GxList* self, void* oldValue, void* newValue, GxDestructor dtor);
	bool (*remove)(GxList* self, void* value);
	bool (*removeByIndex)(GxList* self, int index);
	void (*clean)(GxList* self);
}GxListNamespace;


typedef struct GxMapNamespace {	
	GxMap* (*create)(void);
	void (*destroy)(GxMap* self);	
	Uint32 (*size)(GxMap* self);
	Uint32 (*capacity)(GxMap* self);
	void* (*get)(GxMap* self, const char* key);
	void* (*at)(GxMap* self, Uint32 index);
	void (*set)(GxMap* self, const char* key, void* value, GxDestructor dtor);
	void (*rehash)(GxMap* self, Uint32 capacity);
	void (*remove)(GxMap* self, const char* key);
	void (*removeByIndex)(GxMap* self, Uint32 index);
	void (*clean)(GxMap* self);
}GxMapNamespace;


typedef struct GxContactNamespace {	
	GxElement* (*getColliding)(GxContact* contact);
	GxElement* (*getCollided)(GxContact* contact);
	bool (*isBetween)(GxContact* contact, GxElement* self, GxElement* other);
	bool (*hasElement)(GxContact* contact, GxElement* element);
	bool (*hasDirection)(GxContact* contact, Uint32 direction);
	Uint32 (*getDirection)(GxContact* contact);
	GxElement* (*getOppositeElement)(GxContact* contact, GxElement* self);	
	void (*allowCollision)(GxContact* contact);
	bool (*isElemRightContact)(GxContact* contact, GxElement* self);
	bool (*isElemLeftContact)(GxContact* contact, GxElement* self);
	bool (*isElemDownContact)(GxContact* contact, GxElement* self);
	bool (*isElemUpContact)(GxContact* contact, GxElement* self);
	void (*oneWayPlatform)(GxEvent* e);
	const GxContactConstant RIGHT;
	const GxContactConstant LEFT;
	const GxContactConstant HORIZONTAL;
	const GxContactConstant UP;
	const GxContactConstant DOWN;
	const GxContactConstant VERTICAL;
	const GxContactConstant ALL;
	const Uint32 CMASKNONE;
	const Uint32 CMASKALL;
	const Uint32 CMASKCAMERA;
	const Uint32 CMASKDYNAMIC;
	const Uint32 CMASKFIXED;
}GxContactNamespace;


struct GxStatusNamespace {
	const int NONE;
	const int LOADING;
	const int LOADED;
	const int RUNNING;	
	const int UNLOADING;
};

typedef struct GxSceneNamespace {
		
	GxScene* (*create)(const GxIni* ini);	
	GxData* (*send)(GxScene* receiver, const char* description, GxData* data);	
	void (*addRequestHandler)(GxScene* receiver, const char* request, GxRequestHandler handler);
	void (*delegate)(GxScene* self, const char* sceneReq, GxElemID elem, const char* elemReq);	
	Uint32 (*getPercLoaded)(GxScene* self);
	const char* (*getName)(GxScene* self);
	GxSize (*getSize)(GxScene* self);
	bool (*hasStatus)(GxScene* self, int status);
	int (*getStatus)(GxScene* self);
	GxElement* (*getElem)(GxScene* self, Uint32 id);
	int (*getGravity)(GxScene* self);
	bool (*hasGravity)(GxScene* self);
	GxElement* (*getCamera)(GxScene* self);	
	void (*setGravity)(GxScene* self, int gravity);
	void (*setTimeout)(GxScene* self, int interval, GxHandler callback, void* target);	
	void (*addEventListener)(GxScene* self, int type, GxHandler handler, void* target);
	bool (*removeEventListener)(GxScene* self, int type, GxHandler handler, void* target);	
	const struct GxStatusNamespace* status;
}GxSceneNamespace;


typedef struct GxUtilNamespace {	
	int* (*createInt)(int value);
	Uint32* (*createUint)(Uint32 value);
	bool* (*createBool)(bool value);
	double* (*createDouble)(double value);
	char* (*createString)(const char* value);
	char* (*createStringF)(const char* format, ...);
	char* (*cloneString)(const char* str, char* buffer, unsigned int size);	
	GxArray* (*split)(const char* str, const char* sep);
	char* (*trim)(const char* str, char* buffer, size_t bSize);
	int (*abs)(int value);
	int (*random)(uint32_t* seed, int start, int end);
	void (*printMask)(Uint32 mask);
	SDL_Point (*calcDistance)(const SDL_Point* pointA, const SDL_Point* pointB);
	bool (*assertNullPointer)(const void* ptr); 
	bool (*assertArgument)(bool condition);
	bool (*assertState)(bool condition);
	void* (*assertAlloc)(void* ptr);
	bool (*assertOutOfRange)(bool condition);
} GxUtilNamespace;

//... INSTANCES
extern const GxAppNamespace GxAppNamespaceInstance;
extern const GxArrayNamespace GxArrayNamespaceInstance;
extern const GxButtonNamespace GxButtonNamespaceInstance;
extern const GxElemNamespace GxElemNamespaceInstance;
extern const GxFolderNamespace GxFolderNamespaceInstance;
extern const GxListNamespace GxListNamespaceInstance;
extern const GxMapNamespace GxMapNamespaceInstance;
extern const GxContactNamespace GxContactNamespaceInstance;
extern const GxSceneNamespace GxSceneNamespaceInstance;
extern const GxUtilNamespace GxUtilNamespaceInstance;
 

#endif // !GX_NAMESPACE_H

