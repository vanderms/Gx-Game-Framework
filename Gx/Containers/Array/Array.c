#include "Array.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

//structs
typedef struct sArrayNode {
	void* value;
	sDtor dtor;
} sArrayNode;

typedef struct sArray {   
	sArrayNode* entries;   
    Uint32 size;
    Uint32 capacity; 
} sArray;

//constructor and destructors
sArray* nArrayCreate(void){
	//create self
	sArray* self = malloc(sizeof(sArray));	
	//create attributes    
    self->entries = malloc(sizeof(sArrayNode) * 8);
   nUtilAssertAlloc(self->entries);
    self->capacity = 8;  
    self->size = 0;  
    return self;
}

//...methods
void nArrayDestroy(sArray* self) {
    if (self) {
        for (Uint32 i = 0; i < self->size; i++)
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        free(self->entries);
        free(self);
    }    
}

//accessors and mutators
Uint32 nArraySize(sArray* self){
	return self->size;
}

Uint32 nArrayCapacity(sArray* self){
	return self->capacity;
}

//methods
void* nArrayAt(sArray* self, Uint32 index) {    
    nUtilAssertOutOfRange(index < self->size);
    return self->entries[index].value;
}

void* nArrayLast(sArray* self) {
    if (self->size > 0) {
        return self->entries[self->size - 1].value;
    }
    return NULL;	
}

void nArrayInsert(sArray* self, Uint32 index, void* value, sDtor dtor) {
    
    if (index >= self->size) {
        index = self->size++;
    }
    else if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }

    if (index >= self->capacity) {
        nArrayReserve(self, 2 * self->capacity);
    }
    self->entries[index].value = value;
    self->entries[index].dtor = dtor;
}

void nArrayPush(sArray* self, void* value, sDtor dtor){
    nArrayInsert(self, self->size, value, dtor);
}


void nArrayRemove(sArray* self, Uint32 index) {       
   nUtilAssertOutOfRange(index < self->size);
    if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }
    for (Uint32 i = index; i < self->size - 1; i++) {
        self->entries[i] = self->entries[i + 1];
    }
    self->size--;
}

int nArrayRemoveByValue(sArray* self, void* value) {   
    int64_t index = nArrayIndexOf(self, value);
    if (index != -1) {
        nArrayRemove(self, (Uint32) index);
        return 1;
    } else return 0;
}

int64_t nArrayIndexOf(sArray* self, void* value) {   
    int64_t index = -1;
    for (Uint32 i = 0; i < self->size; i++) {
        if (value == nArrayAt(self, i)) {
            index = i;
            break;
        }
    }
    return index;
}

void nArrayReserve(sArray* self, Uint32 capacity) {   
    if (self->capacity >= capacity) return;
    self->entries = realloc(self->entries, sizeof(sArrayNode) * capacity); 
   nUtilAssertAlloc(self->entries);
    self->capacity = capacity;
}

void nArrayClean(sArray* self) {
    if(self->size > 0 || self->capacity > 8){
        for (Uint32 i = 0; i < self->size; i++) {
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        }
        free(self->entries);
        self->entries = malloc(sizeof(sArrayNode) * 8); 
       nUtilAssertAlloc(self->entries);
        self->capacity = 8;
        self->size = 0;
    }
}


/*
...SORT ALGORITHM:: WORST CASE O(n * log n) BEST CASE O(n)
The main reason there is a sort algorith is that I couldn't use qsort without a global variable.
However, this algorithm is better for arrays already in order or with some order, which is the case
of renderables in Graphics.h.
And this algorithm don't perform much worse, maybe just a bit worse than mergesort, in random arrays.
*/

typedef struct sFragment { 
    Uint32 start; Uint32 end; 
    int order; 
} sFragment;

static inline sFragment* createFragment(Uint32 start, int order) {
    sFragment* self =nUtilAssertAlloc(malloc(sizeof(sFragment)));
    self->start = start;
    self->order = order;
    return self;
}

static inline void reverseFragments(sArray* self, sArray* fragments) {    
    for (Uint32 i = 0; i < fragments->size; i++) {
        sFragment* data = fragments->entries[i].value;
        if (data->order < 0) {
            Uint32 start = data->start, end = data->end;
            while (start < end) {
                sArrayNode temp = self->entries[start];
                self->entries[start] = self->entries[end];
                self->entries[end] = temp;
                start++;
                end--;
            }
        }
    }
}

static inline void mergeFragments(sArray* self, sArray* fragments, sComp comp) {
    
    Uint32 i = 0, step = 1;
    sArrayNode* temp = malloc((self->size) * sizeof(sArrayNode));
   nUtilAssertAlloc(temp);
    
    while (step < fragments->size) {            
        Uint32 second = i + step;            
        if (second >= fragments->size) {
            i = 0;
            step*= 2;
            continue;
        }
        
        sFragment* fs = fragments->entries[i].value;
        sFragment* sn = fragments->entries[second].value;
        Uint32 fsSize = fs->end - fs->start + 1;
        Uint32 snSize = sn->end - sn->start + 1;
        Uint32 total = fsSize + snSize;       
       
        Uint32 fsCounter = 0, snCounter = 0;
            
        for(Uint32 j = 0; j < total; j++) {
                
            if (fsCounter < fsSize && snCounter >= snSize) {
                temp[j] = self->entries[fs->start + fsCounter];
                fsCounter++;
            }
            else if (fsCounter >= fsSize && snCounter < snSize) {
                temp[j] = self->entries[sn->start + snCounter];
                snCounter++;
            }                
            else {
                const void* fsValue = self->entries[fs->start + fsCounter].value;
                const void* snValue = self->entries[sn->start + snCounter].value;
                if (comp(fsValue, snValue) > 0) {
                    temp[j] = self->entries[sn->start + snCounter];
                    snCounter++;
                }
                else {
                    temp[j] = self->entries[fs->start + fsCounter];
                    fsCounter++;
                }
            }
        }

        for (Uint32 j = 0; j < total; j++) {
            self->entries[fs->start + j] = temp[j];
        }
        fs->end = sn->end;
        
        i += (2 * step);            
    }
    free(temp);
}

void nArraySort(sArray* self, sComp comp) {
    
    if (self->size <= 1) { return; }    

    //create fragments
    sArray* fragments = nArrayCreate();   
    nArrayPush(fragments, createFragment(0, 1), free);
  
    //fill fragments    
    for (Uint32 i = 1, counter = 1; i < self->size; i++) {
        sFragment* data = fragments->entries[fragments->size - 1].value;
        int order = comp(self->entries[i].value, self->entries[i -1].value);
        
        if(order == 0) { order = data->order; }         

        if (counter == 1) {
            data->order = order;
            counter++;
            continue;
        }

        if((order > 0 && data->order < 0) || (order < 0 && data->order > 0)){
            nArrayPush(fragments, createFragment(i, 1), free);
            data->end = i - 1;
            counter = 1;
            continue;
        }
        
        counter++;
    }

    //set ->end of the last fragment
    sFragment* lastData = fragments->entries[fragments->size - 1].value;
    lastData->end = self->size - 1;
    
    reverseFragments(self, fragments);
    if(fragments->size > 1) {
        mergeFragments(self, fragments, comp);
    }

    nArrayDestroy(fragments);
}
