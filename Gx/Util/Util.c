#include "../Util/Util.h"
#include "../Containers/Array/Array.h"
#include "../App/App.h"
#include <string.h>
#include <stdio.h>


const Uint32 nUtil_HASH_ELEMENT = 1111251919;
const Uint32 nUtil_HASH_SCENE = 2066184690;
const Uint32 nUtil_HASH_CONTACT = 804125936;

void nUtilPrintMask(Uint32 mask) {
    for (Uint32 i = 0; i < 32; i++) {
        if(i != 0 && i % 8 == 0) putchar('.');
        putchar((1u << i) & mask ? '1' : '0');
    }
    putchar('\n');
}

char* nUtilCreateStringF(const char* format, ...) {
    
    static char buffer[1024]; //1kb
    //...
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);     
    char* value = nUtilCreateString(buffer);    
	return value;
}

int* nUtilCreateInt(int value) {
	int* self = malloc(sizeof(int));
    *self = value;
    return self;
}

Uint32* nUtilCreateUint(Uint32 value) {
	Uint32* self = malloc(sizeof(Uint32));
    nUtilAssertAlloc(self);
    *self = value;
    return self;
}

bool* nUtilCreateBool(bool value) {
    bool* self = malloc(sizeof(bool));
    *self = value;
    return self;
}

double* nUtilCreateDouble(double value) {
	double* self = malloc(sizeof(double));
   nUtilAssertAlloc(self);
    *self = value;
    return self;
}

char* nUtilCreateString(const char* value) {
    nUtilAssertNullPointer(value);
	size_t size = strlen(value) + 1;
	char* self = malloc(sizeof(char) * size);
	nUtilAssertAlloc(self);
	//strncpy is deprecated and strncpy_s does not have portability
	for (size_t i = 0; i < size - 1; i++) self[i] = value[i];
	self[size - 1] = '\0';
	return self;
}

char* nUtilCloneString(const char* str, char* buffer, Uint32 size) {
    unsigned int i = 0;
    while (i < size - 1 && str[i]){
        buffer[i] = str[i];
        i++;
    }
    buffer[i] = '\0';
    return buffer;    
}

sArray* nUtilSplit(const char* str, const char* sep) {
    
    sArray* tokens = nArrayCreate();

    const size_t len = strlen(sep);
    char* next = NULL;
    char* token = nUtilCreateString(str);
    char* memory = token;

    while ((next = strstr(token, sep))){
        next[0] = '\0';
        nArrayPush(tokens, nUtilCreateString(token), free);
        token = next + len;       
    }
    
    nArrayPush(tokens, nUtilCreateString(token), free);
    free(memory);
    
    return tokens;
}

void nUtilSplitAssetPath(const char* path, char* folder, char* asset) {
    char clone[64];
    nUtilCloneString(path, clone, 64);
    char* div = strstr(clone, "/");
    nUtilAssertArgument(div);
    div[0] = '\0';
    nUtilCloneString(clone, folder, 32);
    nUtilCloneString(div + 1, asset, 32);
}


char* nUtilTrim(const char* str, char* buffer, size_t bSize) {    

    size_t len = strlen(str);
    nUtilAssertState(bSize > len);
    
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


SDL_Point nUtilCalcDistance(const SDL_Point* pointA, const SDL_Point* pointB) {
    return (SDL_Point) {
        pointB->x - pointA->x,
        pointB->y - pointA->y
    };
}

int nUtilRandom(uint32_t* seed, int start, int end) {	
	*seed = *seed * 1103515245 + 12345;
	uint32_t partial = *seed % (uint32_t)(end - start + 1);
	int response = (int) partial + start;
	return response;
}

bool nUtilAssertNullPointer(const void* ptr) {
    if (!ptr) {
        nAppRuntimeError("Null pointer error.");
    }
    return ptr;
}

bool nUtilAssertArgument(bool condition){
    if (!condition) {
        nAppRuntimeError("Invalid argument error.");
    }    
    return condition;
}

void* nUtilAssertAlloc(void* ptr){
    if (!ptr) {
        nAppRuntimeError("Failed to allocate memory.");
    }
    return ptr;
}

bool nUtilAssertImplementation(bool condition) {
    if (!condition) {
        nAppRuntimeError("Component not implemented error.");
    }    
    return condition;
}

bool nUtilAssertHash(bool condition) {
    if (!condition) {
        nAppRuntimeError("Invalid hash error.");
    }    
    return condition;
}

bool nUtilAssertOutOfRange(bool condition){
    if (!condition) {
        nAppRuntimeError("Out of range error.");
    }    
    return condition;
}

bool nUtilAssertResourceNotFound(bool condition) {
    if (!condition) {
        nAppRuntimeError("Resource not found error.");
    }    
    return condition;
}

bool nUtilAssertState(bool condition){    
    if (!condition) {
        nAppRuntimeError("Invalid state error.");
    }    
    return condition;
}


void nOnDestroyFreeTarget(sEvent* e) {
    free(e->target);
}

void nOnDestroyDoNothing(sEvent* e) {
    (void) e;
}

bool nComponentIsEmpty_(const sComponent* comp) {
    return (!(comp && (comp->onLoad || comp->onLoopBegin || comp->onUpdate ||
        comp->onRender || comp->onLoopEnd ||
        comp->onUnload || comp->onKeyboard || comp->onMouse ||
        comp->onFinger || comp->onSDLDefault || comp->onPreContact ||
        comp->onContactBegin || comp->onContactEnd || comp->onDestroy
    )));
}

bool nComponentIsIniEmpty_(const sIni* comp) {
    return (!(comp && (comp->onLoad || comp->onLoopBegin || comp->onUpdate ||
        comp->onRender || comp->onLoopEnd ||
        comp->onUnload || comp->onKeyboard || comp->onMouse ||
        comp->onFinger || comp->onSDLDefault || comp->onPreContact ||
        comp->onContactBegin || comp->onContactEnd || comp->onDestroy
    )));
}

sComponent* nComponentCreate_(const sIni* ini) {
    if (nComponentIsIniEmpty_(ini)) {
        return NULL;
    }    
   nUtilAssertArgument(!ini->target || ini->onDestroy);

    sComponent* self =nUtilAssertAlloc(malloc(sizeof(sComponent)));
    
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

sComponent* nComponentCopy_(const sComponent* comp) {
    if (nComponentIsEmpty_(comp)) {
        return NULL;
    }
    sComponent* self = nUtilAssertAlloc(malloc(sizeof(sComponent)));
    *self = *comp;
    self->name = NULL;
    return self;
}

void nComponentDestroy_(sComponent* self) {
    free(self);
}

sHandler nComponentGetHandler_(sComponent* comp, int type){    
    
    if (!comp) {
        return NULL;
    }
    if (type == nEVENT_ON_RENDER) {
        return comp->onRender;
    }
    if (type == nEVENT_ON_PRE_CONTACT) {
        return comp->onPreContact;
    }
    if (type == nEVENT_ON_CONTACT_BEGIN) {
        return comp->onContactBegin;
    }
    if (type == nEVENT_ON_CONTACT_END) {
        return comp->onContactEnd;
    }
    if (type == nEVENT_ON_UPDATE) {
        return comp->onUpdate;
    }   
    if(type == nEVENT_ON_LOOP_BEGIN){
        return comp->onLoopBegin;
    }   
    if (type == nEVENT_ON_LOOP_END) {
        return comp->onLoopEnd;
    }   
    if (type == nEVENT_ON_KEYBOARD) {
        return comp->onKeyboard;
    }
    if (type == nEVENT_ON_MOUSE) {
        return comp->onMouse;
    }
    if (type == nEVENT_ON_FINGER) {
        return comp->onFinger;
    }
    if (type == nEVENT_ON_SDL_DEFAULT) {
        return comp->onSDLDefault;
    }   
    if (type == nEVENT_ON_UNLOAD) {
        return comp->onUnload;
    }
    if (type == nEVENT_ON_LOAD) {
        return comp->onLoad;
    }
    if (type == nEVENT_ON_DESTROY) {
        return comp->onDestroy;
    }
    return NULL;
}
