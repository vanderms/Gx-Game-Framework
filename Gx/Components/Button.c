#include "Components.h"
#include <stdlib.h>
#include <string.h>

typedef struct GxButton {
	Element* base;	
	Uint32 input;
	
	Uint32 status;
	bool clickFlag;		
	SDL_FingerID* fingerID;	
	SDL_TouchID touchID;
	
	int keyCode;	
	Uint32 keyStatus;
	bool keyClickFlag;


} GxButton;

static void setFingerId(GxButton* self, const SDL_FingerID* value){
	
	if (self->fingerID && !value) {
		free(self->fingerID);
		self->fingerID = NULL;				
	}
		
	else if (!self->fingerID && value) {
		self->fingerID = malloc(sizeof(SDL_FingerID));
		*self->fingerID = *value;

	}
	else if (value && self->fingerID) {
		*(self->fingerID) = *value;
	}	
}


static void onLoopBegin(GxEvent* e) {
	GxButton* self = e->target;
	self->status &= component->button->ON;
	self->keyStatus &= component->button->ON;	
}


static void onMouse(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect pos = elem->getPositionOnWindow(self->base);

	if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
		GxPoint p = { e->button.x, e->button.y };			
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= component->button->DOWN;
			self->status |= component->button->ON;
			self->clickFlag = true;
		}
	}

	else if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
		GxPoint p = { e->button.x, e->button.y };		
		if (SDL_PointInRect(&p, &pos)) {	
			self->status |= component->button->UP;
			self->status &= ~component->button->ON;
			if (self->clickFlag) self->status |= component->button->CLICK;
			self->clickFlag = false;
		}
	}

	else if (e->type == SDL_MOUSEMOTION) {
		GxPoint p = { e->motion.x, e->motion.y };
		if (SDL_PointInRect(&p, &pos)) {			
			self->status |= component->button->HOVER;
		}
		else {
			self->status &= ~(component->button->HOVER | component->button->ON);			
			self->clickFlag = false;
		}
	}		
}

static void onFinger(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect pos = elem->getPositionOnWindow(self->base);
	GxSize appSize = app->getWindowSize();

	if (e->type == SDL_FINGERDOWN) {
		GxPoint p = { (int) (e->tfinger.x * appSize.w + 0.5f), (int) (e->tfinger.y * appSize.h + 0.5f) };
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= component->button->DOWN;
			self->status |= component->button->ON;
			self->clickFlag = true;
			setFingerId(self, &e->tfinger.fingerId);
			self->touchID = e->tfinger.touchId;
		}		
	}
	else if (e->type == SDL_FINGERUP) {
			
		if(self->fingerID && e->tfinger.fingerId == *(self->fingerID)){
			GxPoint p = { 
				.x = (int) ((e->tfinger.x * appSize.w) + 0.5f), //0.5f is used to round
				.y = (int) ((e->tfinger.y * appSize.h) + 0.5f) 
			};

			if (SDL_PointInRect(&p, &pos)) {
				self->status |= component->button->UP;	
				self->status &= ~component->button->ON;
				if (self->clickFlag){
					self->status |= component->button->CLICK;
				}
			}
			else {
				self->status &= ~(component->button->HOVER | component->button->ON);							
			}

			self->clickFlag = false;
			setFingerId(self, NULL);
		}
	}
	else if (e->type == SDL_FINGERMOTION) {
		
		GxPoint p = { 
			.x = (int) (e->tfinger.x * appSize.w + 0.5f), 
			.y = (int) (e->tfinger.y * appSize.h + 0.5f) 
		};

		bool isInside = SDL_PointInRect(&p, &pos);

		if(self->fingerID && e->tfinger.fingerId == *(self->fingerID)){
			
			if (!isInside) {
				self->status &= ~(component->button->HOVER | component->button->ON);
				self->clickFlag = false;
				setFingerId(self, NULL);			
			}			
		}
		else if(!self->fingerID){
			if(isInside){
				self->status |= component->button->HOVER;
				self->status |= component->button->ON;
				setFingerId(self, &e->tfinger.fingerId);
			}			
		}		
	}		
}

static void onKeyboard(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	if (self->keyCode == e->key.keysym.sym) {
		if (e->type == SDL_KEYDOWN && !(self->keyStatus & component->button->ON)) {			
			self->keyStatus |= component->button->ON;
			self->keyStatus |= component->button->DOWN;				
		}
		else if (e->type == SDL_KEYUP) {
			self->keyStatus |= component->button->UP;
			self->keyStatus &= ~component->button->ON;
			self->keyStatus |= component->button->CLICK;
		}
	}
}

static void onDestroy(GxEvent* e) {	
	
	GxButton* self = e->target;
	GxScene* sn = GxElemGetScene(self->base);

	scene->removeEventListener(scene, GxEventOnLoopBegin, onLoopBegin, self);
	scene->removeEventListener(scene, GxEventOnDestroy, util->onDestroyFreeTarget, self);

	if (self->input & component->button->KEYBOARD) {
		scene->removeEventListener(scene, GxEventOnKeyboard, onKeyboard, self);
	}
	if (self->input & component->button->FINGER) {
		scene->removeEventListener(scene, GxEventFinger, onFinger, self);
	}
	if (self->input & component->button->MOUSE) {
		scene->removeEventListener(scene, GxEventMouse, onMouse, self);
	}	
	free(self);
}

//acessors
static Uint32 getStatus(Request* req, Response* res) {
	GxButton* self = req->target;
	Uint32 status = component->button->NONE;
	status |= self->input & component->button->SCREEN ? self->status : component->button->NONE;
	status |= self->input & component->button->KEYBOARD ? self->keyStatus : component->button->NONE;
	return status;
}

static bool hasStatus(GxElement* elem, Uint32 status) {
	GxButton* self = GxElemGetChild(elem);
	return (
		((self->input & component->button->SCREEN) && (self->status & status)) ||
		((self->input & component->button->KEYBOARD) && (self->keyStatus & status))
	);
}

//constructor 
static GxElement* createButton(const GxIni* ini, Uint32 inputs, int keyCode) {	
	
	util->assertArgument(!(inputs & component->button->SCREEN) || ini->position);

	GxButton* self = util->assertAlloc(calloc(1, sizeof(GxButton)));
	self->input = inputs;
	self->keyCode = keyCode;
	self->base = elem->create(ini);	
	GxScene* sn = elem->getScene(self->base);

	//add event listeners
	scene->addEventListener(sn, GxEventOnLoopBegin, onLoopBegin, self);
	scene->addEventListener(sn, GxEventOnDestroy, onDestroy, self);

	if (inputs & component->button->KEYBOARD) {
		scene->addEventListener(sn, GxEventOnKeyboard, onKeyboard, self);
	}
	if (inputs & component->button->FINGER) {
		scene->addEventListener(sn, GxEventFinger, onFinger, self);
	}
	if (inputs & component->button->MOUSE) {
		scene->addEventListener(sn, GxEventMouse, onMouse, self);
	}
	return self->base;
}

const ButtonNamespace ComponentButtonInstance = {
	.create = createButton,
	.getStatus = getStatus,
	.hasStatus = hasStatus,
	.KEYBOARD = 1u << 0,
	.FINGER = 1u << 1,
	.MOUSE = 1u << 2,
	.SCREEN = 1U << 1 | 1U << 2,
	.NONE = 0u,
	.ON = 1u << 8,
	.HOVER = 1u << 9,
	.CLICK = 1u << 10,
	.DOWN = 1u << 11,
	.UP = 1u << 12,
};