#include "../Utilities/Util.h"
#include "../Array/Array.h"
#include "../App/App.h"
#include <string.h>
#include <stdio.h>

enum sEventType {
	//lifecycle
	EventOnLoad,
	EventOnLoopBegin,
	EventOnUpdate,
	EventOnRender,
	EventOnLoopEnd,
	EventOnUnload,

	//sdl events
	EventOnKeyboard,
	EventMouse,
	EventFinger,
	EventSDLDefault,

	//contact events
	EventPreContact,
	EventContactBegin,
	EventContactEnd,

	//events not inside ihandlers
	EventTimeout,
	EventOnDestroy,	
    EventTotal,
};

typedef enum sStatus {
	StatusNone,
	StatusLoading,
	StatusLoaded,
	StatusRunning,
	StatusPaused,
	StatusReady,
	StatusUnloading,
} sStatus;


static void printMask(Uint32 mask) {
    for (Uint32 i = 0; i < 32; i++) {
        if(i != 0 && i % 8 == 0) putchar('.');
        putchar((1u << i) & mask ? '1' : '0');
    }
    putchar('\n');
}

static char* createStringF(const char* format, ...) {
    
    static char buffer[1024]; //1kb
    //...
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);     
    char* value = nUtil->createString(buffer);    
	return value;
}

static int* createInt(int value) {
	int* self = malloc(sizeof(int));
    *self = value;
    return self;
}

static Uint32* createUint(Uint32 value) {
	Uint32* self = malloc(sizeof(Uint32));
    nUtil->assertAlloc(self);
    *self = value;
    return self;
}

static bool* createBool(bool value) {
    bool* self = malloc(sizeof(bool));
    *self = value;
    return self;
}

static double* createDouble(double value) {
	double* self = malloc(sizeof(double));
    nUtil->assertAlloc(self);
    *self = value;
    return self;
}

static char* createString(const char* value) {
   nUtil->assertNullPointer(value);
	size_t size = strlen(value) + 1;
	char* self = malloc(sizeof(char) * size);
	nUtil->assertAlloc(self);
	//strncpy is deprecated and strncpy_s does not have portability
	for (size_t i = 0; i < size - 1; i++) self[i] = value[i];
	self[size - 1] = '\0';
	return self;
}

static char* cloneString(const char* str, char* buffer, Uint32 size) {
    unsigned int i = 0;
    while (i < size - 1 && str[i]){
        buffer[i] = str[i];
        i++;
    }
    buffer[i] = '\0';
    return buffer;    
}

static sArray* split(const char* str, const char* sep) {
    
    sArray* tokens = nArray->create();

    const size_t len = strlen(sep);
    char* next = NULL;
    char* token = nUtil->createString(str);
    char* memory = token;

    while ((next = strstr(token, sep))){
        next[0] = '\0';
        nArray->push(tokens, nUtil->createString(token), free);
        token = next + len;       
    }
    
    nArray->push(tokens, nUtil->createString(token), free);
    free(memory);
    
    return tokens;
}

static void splitAssetPath(const char* path, char* folder, char* asset) {
    char clone[64];
    cloneString(path, clone, 64);
    char* div = strstr(clone, "/");
    nUtil->assertArgument(div);
    div[0] = '\0';
    nUtil->cloneString(clone, folder, 32);
    nUtil->cloneString(div + 1, asset, 32);
}


static char* trim(const char* str, char* buffer, size_t bSize) {    

    size_t len = strlen(str);
    nUtil->assertState(bSize > len);
    
    size_t start = 0, index = 0, end = len - 1;
    
    while(str[start] == ' '){ start++;}
    while(str[end] == ' '){ end--; }        
   
    while (index < (end - start) + 1) {
        buffer[index] = str[start + index];
        index++;
    } 
	buffer[index] = '\0';
    return buffer;
}


static SDL_Point calcDistance(const SDL_Point* pointA, const SDL_Point* pointB) {
    return (SDL_Point) {
        pointB->x - pointA->x,
        pointB->y - pointA->y
    };
}

static int random(uint32_t* seed, int start, int end) {	
	*seed = *seed * 1103515245 + 12345;
	uint32_t partial = *seed % (uint32_t)(end - start + 1);
	int response = (int) partial + start;
	return response;
}

static bool assertNullPointer(const void* ptr) {
    if (!ptr) {
        nApp->runtimeError("Null pointer error.");
    }
    return ptr;
}

static bool assertArgument(bool condition){
    if (!condition) {
        nApp->runtimeError("Invalid argument error.");
    }    
    return condition;
}

static void* assertAlloc(void* ptr){
    if (!ptr) {
        nApp->runtimeError("Failed to allocate memory.");
    }
    return ptr;
}

static bool assertImplementation(bool condition) {
    if (!condition) {
        nApp->runtimeError("Component not implemented error.");
    }    
    return condition;
}

static bool assertHash(bool condition) {
    if (!condition) {
        nApp->runtimeError("Invalid hash error.");
    }    
    return condition;
}

static bool assertOutOfRange(bool condition){
    if (!condition) {
        nApp->runtimeError("Out of range error.");
    }    
    return condition;
}

static bool assertResourceNotFound(bool condition) {
    if (!condition) {
        nApp->runtimeError("Resource not found error.");
    }    
    return condition;
}

static bool assertState(bool condition){    
    if (!condition) {
        nApp->runtimeError("Invalid state error.");
    }    
    return condition;
}




const struct sUtilNamespace*  nUtil = &(struct sUtilNamespace){
	.createInt = createInt,
	.createUint = createUint,
	.createBool = createBool,
	.createDouble = createDouble,
	.createString = createString,
	.createStringF = createStringF,
	.cloneString = cloneString,
	.split = split,
	.trim = trim,	
	.random = random,
	.printMask = printMask,
	.calcDistance = calcDistance,
	.assertNullPointer = assertNullPointer,
    .assertImplementation = assertImplementation,
    .assertResourceNotFound = assertResourceNotFound,
	.assertArgument = assertArgument,
    .assertHash = assertHash,
	.assertState =assertState,
	.assertAlloc = assertAlloc,
	.assertOutOfRange = assertOutOfRange,	
	.splitAssetPath = splitAssetPath,
    
    .status = &(struct sUtilStatusNamespace){
		.NONE =StatusNone,
		.LOADING = StatusLoading,
		.LOADED = StatusLoaded,
		.RUNNING = StatusRunning,
        .PAUSED = StatusPaused,
        .READY = StatusReady,
		.UNLOADING = StatusUnloading,
	},	

    .hash = &(struct sUtilHash) {
        .ELEMENT = 1111251919,	
	    .SCENE = 2066184690,
	    .CONTACT = 804125936,
    },
};

static void onDestroyFreeTarget(sEvent* e) {
    free(e->target);
}

static void onDestroyDoNothing(sEvent* e) {
    (void) e;
}

static bool isComponentEmpty(const sComponent* comp) {
    return (!(comp && (comp->onLoad || comp->onLoopBegin || comp->onUpdate ||
        comp->onRender || comp->onLoopEnd ||
        comp->onUnload || comp->onKeyboard || comp->onMouse ||
        comp->onFinger || comp->onSDLDefault || comp->onPreContact ||
        comp->onContactBegin || comp->onContactEnd || comp->onDestroy
    )));
}

static bool isIniComponentEmpty(const sIni* comp) {
    return (!(comp && (comp->onLoad || comp->onLoopBegin || comp->onUpdate ||
        comp->onRender || comp->onLoopEnd ||
        comp->onUnload || comp->onKeyboard || comp->onMouse ||
        comp->onFinger || comp->onSDLDefault || comp->onPreContact ||
        comp->onContactBegin || comp->onContactEnd || comp->onDestroy
    )));
}

static sComponent* createComponent(const sIni* ini) {
    if (isIniComponentEmpty(ini)) {
        return NULL;
    }    
    nUtil->assertArgument(!ini->target || ini->onDestroy);

    sComponent* self = nUtil->assertAlloc(malloc(sizeof(sComponent)));
    
    self->target = ini->target;
    self->name = NULL;
    self->onLoad = ini->onLoad;
    self->onLoopBegin = ini->onLoopBegin;
    self->onUpdate = ini->onUpdate;
    self->onRender = ini->onRender;
    self->onLoopEnd = ini->onLoopEnd;
    self->onUnload = ini->onUnload;
    self->onKeyboard = ini->onKeyboard;
    self->onMouse = ini->onMouse;
    self->onFinger = ini->onFinger;
    self->onSDLDefault = ini->onSDLDefault;
    self->onPreContact = ini->onPreContact;
    self->onContactBegin = ini->onContactBegin;
    self->onContactEnd = ini->onContactEnd;
    self->onDestroy = ini->onDestroy;   
    return self;
}
static sComponent* copyComponent(const sComponent* comp) {
    if (isComponentEmpty(comp)) {
        return NULL;
    }
    sComponent* self = nUtil->assertAlloc(malloc(sizeof(sComponent)));
    *self = *comp;
    self->name = NULL;
    return self;
}

static void destroyComponent(sComponent* self) {
    free(self);
}

static sHandler getHandler(sComponent* comp, int type){    
    
    if (!comp) {
        return NULL;
    }
    if (type == EventOnRender) {
        return comp->onRender;
    }
    if (type == EventPreContact) {
        return comp->onPreContact;
    }
    if (type == EventContactBegin) {
        return comp->onContactBegin;
    }
    if (type == EventContactEnd) {
        return comp->onContactEnd;
    }
    if (type == EventOnUpdate) {
        return comp->onUpdate;
    }   
    if(type == EventOnLoopBegin){
        return comp->onLoopBegin;
    }   
    if (type == EventOnLoopEnd) {
        return comp->onLoopEnd;
    }   
    if (type == EventOnKeyboard) {
        return comp->onKeyboard;
    }
    if (type == EventMouse) {
        return comp->onMouse;
    }
    if (type == EventFinger) {
        return comp->onFinger;
    }
    if (type == EventSDLDefault) {
        return comp->onSDLDefault;
    }   
    if (type == EventOnUnload) {
        return comp->onUnload;
    }
    if (type == EventOnLoad) {
        return comp->onLoad;
    }
    if (type == EventOnDestroy) {
        return comp->onDestroy;
    }
    return NULL;
};


const struct sComponentNamespace* nComponent = &(struct sComponentNamespace) {
    .create = createComponent,
    .copy = copyComponent,
    .destroy = destroyComponent,
    .isComponentEmpty = isComponentEmpty,
    .isIniComponentEmpty = isIniComponentEmpty,
    .onDestroyFreeTarget = onDestroyFreeTarget,
	.onDestroyDoNothing = onDestroyDoNothing,
    .getHandler = getHandler,
    .ON_LOAD = EventOnLoad,
	.ON_LOOP_BEGIN = EventOnLoopBegin,
	.ON_UPDATE = EventOnUpdate,
	.ON_RENDER = EventOnRender,
	.ON_LOOP_END = EventOnLoopEnd,
	.ON_UNLOAD = EventOnUnload,	
	.ON_KEYBOARD = EventOnKeyboard,
	.ON_MOUSE = EventMouse,
	.ON_FINGER = EventFinger,
	.ON_SDL_DEFAULT = EventSDLDefault,
	.ON_PRE_CONTACT = EventPreContact,
	.ON_CONTACT_BEGIN = EventContactBegin,
	.ON_CONTACT_END = EventContactEnd,
	.ON_TIMEOUT = EventTimeout,
	.ON_DESTROY = EventOnDestroy,	
    .TOTAL = EventTotal
};