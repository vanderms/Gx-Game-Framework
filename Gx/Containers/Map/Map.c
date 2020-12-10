#include "Map.h"
#include <stdint.h>
#include "../List/List.h"

#define SWAP(a, b) {void* temp = a; a = b; b = temp; }

typedef struct GxInt { int value; }GxInt;

static inline GxInt* createInt(int value) {
	GxInt* self = malloc(sizeof(GxInt));	
	nUtilAssertAlloc(self);
	self->value = value;
	return self;
}

/*************************************************************************************************************
 *  *** AUXILIARY METHODS ***
 *************************************************************************************************************/ 
 typedef struct Entry {
	//data
	void* value;
	sDtor dtor;
	//cross reference	
	GxInt* reference;
} Entry;


typedef struct sMap {
	Uint32 size;
	Uint32 capacity;	
	char** keys;
	Entry* entries;
	sList** table;
} sMap;

static inline int calcHash(const char* str, Uint32 max) {	
	uint64_t hash = 5381;
	int c;
	while ((c = *str++)) hash = ((hash << 5) + hash) + (uint64_t) c;
	return (int) (hash % (uint64_t) max);
}

/*************************************************************************************************************
 *  *** CONSTRUCTOR AND DESTRUCTOR ***
 *************************************************************************************************************/

sMap* nMapCreate() {
	sMap* self = malloc(sizeof(sMap));
	nUtilAssertAlloc(self);
	self->size = 0;
	self->capacity = 16;
	
	//create table
	self->table = malloc(self->capacity * sizeof(sList*));	
	nUtilAssertAlloc(self->table);
	self->entries = malloc(self->capacity * sizeof(Entry));
	nUtilAssertAlloc(self->entries);
	self->keys = malloc(self->capacity * sizeof(char*));
	nUtilAssertAlloc(self->keys);
	
	for (Uint32 i = 0; i < self->capacity; i++) {
		self->table[i] = nListCreate();
	}
	//then return
	return self;
}


void nMapDestroy(sMap* self) {
	if (self) {
		for (Uint32 i = 0; i < self->capacity; i++) {
			nListDestroy(self->table[i]);
			if(i >= self->size) continue;
			free(self->keys[i]);
			if(self->entries[i].dtor){
				self->entries[i].dtor(self->entries[i].value);
			}
			free(self->entries[i].reference);
		}

		free(self->table);
		free(self->keys);
		free(self->entries);
		free(self);
	}	
}

/*************************************************************************************************************
 *  *** MAP METHODS ***
 *************************************************************************************************************/

Uint32 nMapSize(sMap* self) {
	return self->size;
}

Uint32 nMapCapacity(sMap* self) {
	return self->capacity;
}

void* nMapGet(sMap* self, const char* key) {	
	
	sList* bucket = self->table[calcHash(key, self->capacity)];
	for (char* k = nListBegin(bucket); k != NULL; k = nListNext(bucket)) {
		GxInt* reference = nListNext(bucket);
		if (strcmp(k, key) == 0) {
			return self->entries[reference->value].value;
		}
	}
	return NULL;
}

void* nMapAt(sMap* self, Uint32 index) {
	nUtilAssertOutOfRange(index < self->size);
	return self->entries[index].value;
}

void nMapSet(sMap* self, const char* key, void* value, sDtor dtor) {
	
	if (self->size >= self->capacity){
		nMapRehash(self, self->capacity * 2);
	}	

	bool contains = false;
	sList* bucket = self->table[calcHash(key, self->capacity)];
	
	for (char* lsKey = nListBegin(bucket); lsKey != NULL; 
		lsKey = nListNext(bucket))
	{		
		GxInt* index = nListNext(bucket);
		if (strcmp(lsKey, key) == 0) {
			Entry* entry = &self->entries[index->value];
			if(entry->dtor) entry->dtor(entry->value);
			entry->value = value;
			entry->dtor = dtor;
			contains = true;				
			break;
		}
	}	

	if (!contains) {	
		//fill bucket
		char* k = nUtilCreateString(key);
		GxInt* index = createInt(self->size);
		nListPush(bucket, k, NULL);
		nListPush(bucket, index, NULL);

		//fill entries and keys
		self->entries[self->size].value = value;
		self->entries[self->size].dtor = dtor;		
		self->entries[self->size].reference = index;
		self->keys[self->size] = k;
				
		//change size
		self->size++;
	}
}


void nMapRehash(sMap* self, Uint32 capacity) {

	if (self->capacity >= capacity) return;

	//create replacement table
	sList** table = malloc(capacity * sizeof(sList*));
	nUtilAssertAlloc(table);
	
	for (Uint32 i = 0; i < capacity; i++) {
		table[i] = nListCreate();
	}

	for (Uint32 i = 0; i < self->capacity; i++) {

		sList* source = self->table[i];		
		
		for (char* key = nListBegin(source); key != NULL; key = nListNext(source)) {
			int* index = nListNext(source);
			sList* bucket = table[calcHash(key, capacity)];
			nListPush(bucket, key, NULL);
			nListPush(bucket, index, NULL);
		}		
	}
	
	//swap table and realloc keys and entries
	SWAP(self->table, table)	
	self->keys = realloc(self->keys, capacity * sizeof(char*));
	nUtilAssertAlloc(self->keys);
	self->entries = realloc(self->entries, capacity* sizeof(Entry));
	nUtilAssertAlloc(self->entries);
	
	//destroy previous table
	for (Uint32 i = 0; i < self->capacity; i++) {
		nListDestroy(table[i]);
	}
	free(table);
	//set new capacity
	self->capacity = capacity;		
}

void nMapRemove(sMap* self, const char* key) {
	
	GxInt* index = NULL;
	bool contains = false;
	sList* bucket = self->table[calcHash(key, self->capacity)];
	
	for (char* k = nListBegin(bucket); k != NULL; k = nListNext(bucket)){		
		index = nListNext(bucket);
		if (strcmp(k, key) == 0) {	
			contains = true;			
			nListRemove(bucket, k);
			nListRemove(bucket, index);
			break;
		}	
	}

	if (contains) {
		//remove value, and free key 
		if (self->entries[index->value].dtor) {
 			self->entries[index->value].dtor(self->entries[index->value].value);
		}
		free(self->keys[index->value]);

		//set new size
		--self->size;

		//update entries and keys
		for (Uint32 i = index->value; i < self->size; i++) {
			self->keys[i] = self->keys[i + 1];
			self->entries[i] = self->entries[i + 1];
			self->entries[i].reference->value = i;
		}

		//free index
		free(index);
	}
}

void nMapRemoveByIndex(sMap* self, Uint32 index) {
	nUtilAssertOutOfRange(index < self->size);
	char* key = self->keys[index];
	nMapRemove(self, key);
}

void nMapClean(sMap* self) {
	
	//first create a empty map
	sMap* empty = nMapCreate();	
		
	//then, swap table and keys
	SWAP(self->table, empty->table)
	SWAP(self->keys, empty->keys)
	SWAP(self->entries, empty->entries)

	//Set capacity and size
	Uint32 temp = self->capacity;
	self->capacity = empty->capacity;
	empty->capacity = temp;
	empty->size = self->size;
	self->size = 0;

	//finally destroy empty
	nMapDestroy(empty);
}
