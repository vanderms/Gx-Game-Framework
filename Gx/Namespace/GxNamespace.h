#ifndef GX_H
#define GX_H
#include "../Public/GxPublic.h"
#include "../Ini/Ini.h"
#include "../App/App.h"

//... NAMESPACES


struct sEventNamespace {
	const int LOAD;
	const int LOOP_BEGIN;
	const int UPDATE;
	const int ON_RENDER;	
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










typedef struct GxElemNamespace {
	
	//...sElement
	sElement* (*create)(const sIni* ini);	
	sElement* (*createTilemap)(const char* tilePath, const sIni* ini);	
	void (*remove)(sElement* self);	
	void* (*getTarget)(sElement* self);
	Uint32 (*getID)(sElement* self);
	GxScene* (*getScene)(sElement* self);

	const SDL_Rect* (*getPos)(sElement* self);
	void (*setPos)(sElement* self, SDL_Rect pos);
	sPoint (*getCenter)(sElement* self);
	
	const char* (*getClassName)(sElement* self);
	bool (*hasClass)(sElement* self, const char* type);
	
	bool (*hasHandler)(sElement* self, int type);
	GxHandler (*getHandler)(sElement* self, int type);	
	void (*addRequestHandler)(sElement* self, const char* request, GxRequestHandler handler);
	
	bool (*isPhysical)(sElement* self);
	bool (*isRenderable)(sElement* self);
	bool (*hasDynamicBody)(sElement* self);
	bool (*hasFixedBody)(sElement* self);
	bool (*hasRelativePosition)(sElement* self);
	bool (*hasAbsolutePosition)(sElement* self);	
	
	void (*setChild)(sElement* self, void* child);
	void* (*getChild)(sElement* self);
	
	//...RigidBody
	bool (*isOnGround)(sElement* self);

	Uint32 (*getCmask)(sElement* self);
	void (*setCmask)(sElement* self, Uint32 mask);

	int (*getPreference)(sElement* self);
	void (*setPreference)(sElement* self, int value);

	bool (*hasFriction)(sElement* self);
	void (*setFriction)(sElement* self, bool value);

	sVector (*getVelocity)(sElement* self);
	void (*setVelocity)(sElement* self, sVector velocity);

	int (*getVely)(sElement* self);
	void (*setVely)(sElement* self, int vel);

	int (*getVelx)(sElement* self);
	void (*setVelx)(sElement* self, int vel);

	void (*accelerate)(sElement* self, double x, double y);

	bool (*isMoving)(sElement* self);

	double (*getElasticity)(sElement* self);
	void (*setElasticity)(sElement* self, double elasticity);

	double (*getRestitution)(sElement* self);
	void (*setRestitution)(sElement* self, double restitution);

	int (*getMaxgvel)(sElement* self);
	void (*setMaxgvel)(sElement* self, int value);

	sArray* (*getContacts)(sElement* self, int types);	

	sVector (*move)(sElement* self, sVector vector, bool force);
	void (*moveTo)(sElement* self, sPoint pos, bool force);

	//..Renderable	
	int (*getZIndex)(sElement* self);
	void (*setZIndex)(sElement* self, int index);

	Uint8 (*getOpacity)(sElement* self);
	void (*setOpacity)(sElement* self, Uint8 value);

	int (*getOrientation)(sElement* self);
	void (*setOrientation)(sElement* self, int value);

	const char* (*getImage)(sElement* self);
	void (*setImage)(sElement* self, const char* path);

	const char* (*getAnimation)(sElement* self);
	void (*setAnimation)(sElement* self, const char* path);

	const char* (*getAlignment)(sElement* self);
	void (*setAlignment)(sElement* self, const char* value);

	bool (*isHidden)(sElement* self);
	void (*hide)(sElement* self);
	void (*show)(sElement* self);

	double (*getAngle)(sElement* self);
	void (*setAngle)(sElement* self, double angle);

	double (*getProportion)(sElement* self);
	void (*setProportion)(sElement* self, double proportion);
	void (*setToFit)(sElement* self, const char* axis);
		
	const SDL_Color* (*getBackgroundColor)(sElement* self);
	void (*setBackgroundColor)(sElement* self, const char* color);

	int (*getBorderSize)(sElement* self);	
	const SDL_Color* (*getBorderColor)(sElement* self);
	void (*setBorder)(sElement* self, const char* border);

	void (*setText)(sElement* self, const char* text, ...);	
	const char* (*getText)(sElement* self);
	
	void (*setFontSize)(sElement* self, int size);
	int (*getFontSize)(sElement* self);

	void (*setFont)(sElement* self, const char* font);
	const char* (*getFont)(sElement* self);

	const SDL_Color* (*getColor)(sElement* self);
	void (*setColor)(sElement* self, const char* color);	

	SDL_Rect (*getPositionOnWindow)(sElement* self);
		
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
	void (*createTilemap)(const char* folderName, const char* name,
		const char* group, GxSize size, GxMatrix matrix, int* sequence
	);
	void(*removeAsset)(const char* path);
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
	sElement* (*getColliding)(GxContact* contact);
	sElement* (*getCollided)(GxContact* contact);
	bool (*isBetween)(GxContact* contact, sElement* self, sElement* other);
	bool (*hasElement)(GxContact* contact, sElement* element);
	bool (*hasDirection)(GxContact* contact, Uint32 direction);
	Uint32 (*getDirection)(GxContact* contact);
	sElement* (*getOppositeElement)(GxContact* contact, sElement* self);	
	void (*allowCollision)(GxContact* contact);
	bool (*isElemRightContact)(GxContact* contact, sElement* self);
	bool (*isElemLeftContact)(GxContact* contact, sElement* self);
	bool (*isElemDownContact)(GxContact* contact, sElement* self);
	bool (*isElemUpContact)(GxContact* contact, sElement* self);
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
		
	GxScene* (*create)(const sIni* ini);		
	void (*addRequestHandler)(GxScene* receiver, const char* request, GxRequestHandler handler);	
	Uint32 (*getPercLoaded)(GxScene* self);
	const char* (*getName)(GxScene* self);
	GxSize (*getSize)(GxScene* self);
	bool (*hasStatus)(GxScene* self, int status);
	int (*getStatus)(GxScene* self);
	sElement* (*getElem)(GxScene* self, Uint32 id);
	int (*getGravity)(GxScene* self);
	bool (*hasGravity)(GxScene* self);
	sElement* (*getCamera)(GxScene* self);	
	void (*setGravity)(GxScene* self, int gravity);
	void (*setTimeout)(GxScene* self, int interval, GxHandler callback, void* target);	
	void (*addEventListener)(GxScene* self, int type, GxHandler handler, void* target);
	bool (*removeEventListener)(GxScene* self, int type, GxHandler handler, void* target);	
	const struct GxStatusNamespace* status;
}GxSceneNamespace;




//... INSTANCES
extern const GxElemNamespace GxElemNamespaceInstance;
extern const GxFolderNamespace GxFolderNamespaceInstance;
extern const GxListNamespace GxListNamespaceInstance;
extern const GxMapNamespace GxMapNamespaceInstance;
extern const GxContactNamespace GxContactNamespaceInstance;
extern const GxSceneNamespace GxSceneNamespaceInstance;
#endif // !GX_NAMESPACE_H

