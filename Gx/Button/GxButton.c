#include "../Utilities/GxUtil.h"
#include "../Button/GxButton.h"
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../Array/GxArray.h"
#include "../Scene/GxScene.h"
#include "../App/GxApp.h"
#include <stdlib.h>
#include <string.h>


typedef struct GxButton {
	GxElement* base;	
	Uint32 input;
	
	Uint32 status;
	bool clickFlag;
	
	int keyCode;	
	Uint32 keyStatus;
	bool keyClickFlag;
} GxButton;

static void onLoopBegin(GxEvent* e) {
	GxButton* self = e->target;
	self->status &= GxButtonOn;
	self->keyStatus &= GxButtonOn;
}


static void onMouse(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect pos = GxGetElemPositionOnWindow(self->base);

	if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
		GxPoint p = { e->button.x, e->button.y };			
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= GxButtonDown;
			self->status |= GxButtonOn;
			self->clickFlag = true;
		}
	}

	else if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
		GxPoint p = { e->button.x, e->button.y };		
		if (SDL_PointInRect(&p, &pos)) {	
			self->status |= GxButtonUp;
			self->status &= ~GxButtonOn;
			if (self->clickFlag) self->status |= GxButtonClick;
			self->clickFlag = false;
		}
	}

	else if (e->type == SDL_MOUSEMOTION) {
		GxPoint p = { e->motion.x, e->motion.y };
		if (SDL_PointInRect(&p, &pos)) {			
			self->status |= GxButtonHover;
		}
		else {
			self->status &= ~(GxButtonHover | GxButtonOn);			
			self->clickFlag = false;
		}
	}		
}

static void onFinger(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect pos = GxGetElemPositionOnWindow(self->base);
	GxSize appSize = GxGetWindowSize();

	if (e->type == SDL_FINGERDOWN) {
		GxPoint p = { (int) (e->tfinger.x * appSize.w + 0.5f), (int) (e->tfinger.y * appSize.h + 0.5f) };
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= GxButtonDown;
			self->status |= GxButtonOn;
			self->clickFlag = true;
		}
	}
	else if (e->type == SDL_FINGERUP) {
		GxPoint p = { (int) (e->tfinger.x * appSize.w + 0.5f), (int) (e->tfinger.y * appSize.h + 0.5f) };
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= GxButtonUp;	
			self->status &= ~GxButtonOn;
			if (self->clickFlag) self->status |= GxButtonClick;
			self->clickFlag = false;
		}
	}
	else if (e->type == SDL_FINGERMOTION) {
		GxPoint p = { (int) (e->tfinger.x * appSize.w + 0.5f), (int) (e->tfinger.y * appSize.h + 0.5f) };		
		SDL_Rect area = { pos.x - 25, pos.y - 25, pos.w + 50, pos.h + 50 };
		bool inside_pos = SDL_PointInRect(&p, &pos);
		bool inside_area = SDL_PointInRect(&p, &area);
		if (inside_pos) {
			self->status |= GxButtonHover;
			self->status |= GxButtonOn;
		}
		else if (!inside_pos && inside_area) {
			self->status &= ~(GxButtonHover | GxButtonOn);
			self->clickFlag = false;
		}
	}		
}

static void onKeyboard(GxEvent* ev) {
	
	GxButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	if (self->keyCode == e->key.keysym.sym) {
		if (e->type == SDL_KEYDOWN && !(self->keyStatus & GxButtonOn)) {			
			self->keyStatus |= GxButtonOn;
			self->keyStatus |= GxButtonDown;				
		}
		else if (e->type == SDL_KEYUP) {
			self->keyStatus |= GxButtonUp;
			self->keyStatus &= ~GxButtonOn;
			self->keyStatus |= GxButtonClick;
		}
	}
}

static void onDestroy(GxEvent* e) {	
	
	GxButton* self = e->target;
	GxScene* scene = GxElemGetScene(self->base);

	GxSceneRemoveEventListener(scene, GxEventOnLoopBegin, onLoopBegin, self);
	GxSceneRemoveEventListener(scene, GxEventOnDestroy, GxFreeTarget, self);

	if (self->input & GxButtonKeyboard) {
		GxSceneRemoveEventListener(scene, GxEventOnKeyboard, onKeyboard, self);
	}
	if (self->input & GxButtonFinger) {
		GxSceneRemoveEventListener(scene, GxEventFinger, onFinger, self);
	}
	if (self->input & GxButtonMouse) {
		GxSceneRemoveEventListener(scene, GxEventMouse, onMouse, self);
	}	
	free(self);
}


//constructor 
GxElement* GxCreateButton(const GxIni* ini, Uint32 inputs, int keyCode) {	
	
	GxAssertInvalidArgument(!(inputs & GxButtonScreen) || ini->position);

	GxButton* self = calloc(1, sizeof(GxButton));
	GxAssertAllocationFailure(self);
	self->input = inputs;
	self->keyCode = keyCode;
	self->base = GxCreateElement(ini);
	GxElemSetChild(self->base, self);
	GxScene* scene = GxElemGetScene(self->base);

	//add event listeners
	GxSceneAddEventListener(scene, GxEventOnLoopBegin, onLoopBegin, self);
	GxSceneAddEventListener(scene, GxEventOnDestroy, onDestroy, self);

	if (inputs & GxButtonKeyboard) {
		GxSceneAddEventListener(scene, GxEventOnKeyboard, onKeyboard, self);
	}
	if (inputs & GxButtonFinger) {
		GxSceneAddEventListener(scene, GxEventFinger, onFinger, self);
	}
	if (inputs & GxButtonMouse) {
		GxSceneAddEventListener(scene, GxEventMouse, onKeyboard, self);
	}
	return self->base;
}

//acessors
Uint32 GxButtonGetStatus(GxElement* elem) {
	GxButton* self = GxElemGetChild(elem);
	Uint32 status = GxButtonNone;
	status |= self->input & GxButtonScreen ? self->status : GxButtonNone;
	status |= self->input & GxButtonKeyboard ? self->keyStatus : GxButtonNone;
	return status;
}

bool GxButtonHasStatus(GxElement* elem, Uint32 status) {
	GxButton* self = GxElemGetChild(elem);
	return (
		((self->input & GxButtonScreen) && (self->status & status)) ||
		((self->input & GxButtonKeyboard) && (self->keyStatus & status))
	);
}
