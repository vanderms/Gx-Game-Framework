#include "Button.h"

typedef struct sButton {
	sElement* base;	
	Uint32 input;
	
	Uint32 status;
	bool clickFlag;		
	SDL_FingerID* fingerID;	
	SDL_TouchID touchID;
	
	int keyCode;	
	Uint32 keyStatus;
	bool keyClickFlag;

} sButton;


const Uint32 nBUTTON_KEYBOARD = 1u << 0;
const Uint32 nBUTTON_FINGER = 1u << 1;
const Uint32 nBUTTON_MOUSE = 1u << 2;
const Uint32 nBUTTON_SCREEN = 1U << 1 | 1U << 2;
const Uint32 nBUTTON_NONE = 0u;
const Uint32 nBUTTON_ON = 1u << 8;
const Uint32 nBUTTON_HOVER = 1u << 9;
const Uint32 nBUTTON_CLICK = 1u << 10;
const Uint32 nBUTTON_DOWN = 1u << 11;
const Uint32 nBUTTON_UP = 1u << 12;


static void setFingerId(sButton* self, const SDL_FingerID* value){
	
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


static void onLoopBegin(sEvent* e) {
	sButton* self = e->target;
	self->status &= (nBUTTON_ON | nBUTTON_HOVER);
	self->keyStatus &= nBUTTON_ON;	
}


static void onMouse(sEvent* ev) {
	
	sButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect posOnCamera = nElemCalcPosOnCamera(self->base);
	SDL_Rect pos = {0, 0, 0, 0};
	nAppCalcDest(&posOnCamera, &pos);

	if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
		sPoint p = { e->button.x, e->button.y };			
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= nBUTTON_DOWN;
			self->status |= nBUTTON_ON;
			self->clickFlag = true;
		}
	}

	else if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
		sPoint p = { e->button.x, e->button.y };		
		if (SDL_PointInRect(&p, &pos)) {	
			self->status |= nBUTTON_UP;
			self->status &= ~nBUTTON_ON;
			if (self->clickFlag) self->status |= nBUTTON_CLICK;
			self->clickFlag = false;
		}
	}

	else if (e->type == SDL_MOUSEMOTION) {
		sPoint p = { e->motion.x, e->motion.y };
		if (SDL_PointInRect(&p, &pos)) {			
			self->status |= nBUTTON_HOVER;
		}
		else {
			self->status &= ~(nBUTTON_HOVER | nBUTTON_ON);			
			self->clickFlag = false;
		}
	}		
}

static void onFinger(sEvent* ev) {
	
	sButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	SDL_Rect pos = nElemCalcPosOnCamera(self->base);

	sSize appSize = nAppLogicalSize();

	if (e->type == SDL_FINGERDOWN) {
		sPoint p = { (int) (e->tfinger.x * appSize.w + 0.5f), (int) (e->tfinger.y * appSize.h + 0.5f) };
		if (SDL_PointInRect(&p, &pos)) {
			self->status |= nBUTTON_DOWN;
			self->status |= nBUTTON_ON;
			self->clickFlag = true;
			setFingerId(self, &e->tfinger.fingerId);
			self->touchID = e->tfinger.touchId;
		}		
	}
	else if (e->type == SDL_FINGERUP) {
			
		if(self->fingerID && e->tfinger.fingerId == *(self->fingerID)){
			sPoint p = { 
				.x = (int) ((e->tfinger.x * appSize.w) + 0.5f), //0.5f is used to round
				.y = (int) ((e->tfinger.y * appSize.h) + 0.5f) 
			};

			if (SDL_PointInRect(&p, &pos)) {
				self->status |= nBUTTON_UP;	
				self->status &= ~nBUTTON_ON;
				if (self->clickFlag){
					self->status |= nBUTTON_CLICK;
				}
			}
			else {
				self->status &= ~(nBUTTON_HOVER | nBUTTON_ON);							
			}

			self->clickFlag = false;
			setFingerId(self, NULL);
		}
	}
	else if (e->type == SDL_FINGERMOTION) {
		
		sPoint p = { 
			.x = (int) (e->tfinger.x * appSize.w + 0.5f), 
			.y = (int) (e->tfinger.y * appSize.h + 0.5f) 
		};

		bool isInside = SDL_PointInRect(&p, &pos);

		if(self->fingerID && e->tfinger.fingerId == *(self->fingerID)){
			
			if (!isInside) {
				self->status &= ~(nBUTTON_HOVER | nBUTTON_ON);
				self->clickFlag = false;
				setFingerId(self, NULL);			
			}			
		}
		else if(!self->fingerID){
			if(isInside){
				self->status |= nBUTTON_HOVER;
				self->status |= nBUTTON_ON;
				setFingerId(self, &e->tfinger.fingerId);
			}			
		}		
	}		
}

static void onKeyboard(sEvent* ev) {
	
	sButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	if (self->keyCode == e->key.keysym.sym) {
		if (e->type == SDL_KEYDOWN && !(self->keyStatus & nBUTTON_ON)) {			
			self->keyStatus |= nBUTTON_ON;
			self->keyStatus |= nBUTTON_DOWN;				
		}
		else if (e->type == SDL_KEYUP) {
			self->keyStatus |= nBUTTON_UP;
			self->keyStatus &= ~nBUTTON_ON;
			self->keyStatus |= nBUTTON_CLICK;
		}
	}
}

Uint32 nButtonGetStatus(sElement* base){
	sButton* self = nElemGetComponent(base, "nButton");
	Uint32 status = nBUTTON_NONE;
	status |= self->input & nBUTTON_SCREEN ? self->status : nBUTTON_NONE;
	status |= self->input & nBUTTON_KEYBOARD ? self->keyStatus : nBUTTON_NONE;
	return status;
}

bool nButtonHasStatus(sElement* base, Uint32 status) {
	sButton* self = nElemGetComponent(base, "nButton");
	return (
		((self->input & nBUTTON_SCREEN) && (self->status & status)) ||
		((self->input & nBUTTON_KEYBOARD) && (self->keyStatus & status))
	);
}

//constructor 
sElement* nButtonCreate(sElement* base, Uint32 inputs, int keyCode) {	
	
	nUtilAssertArgument(!(inputs & nBUTTON_SCREEN) || nElemPosition(base));

	sButton* self =nUtilAssertAlloc(calloc(1, sizeof(sButton)));
	self->input = inputs;
	self->keyCode = keyCode;
	self->base = base;

	sHandler keyboardHandler = (inputs & nBUTTON_KEYBOARD) ? onKeyboard : NULL;
	sHandler mouseHandler = (inputs & nBUTTON_MOUSE) ? onMouse : NULL;
	sHandler fingerHandler = (inputs & nBUTTON_MOUSE) ? onFinger : NULL;
	
	nElemAddComponent(base, &(sComponent){
		.name = "nButton",
		.target = self,
		.onLoopBegin = onLoopBegin,
		.onKeyboard = keyboardHandler,
		.onMouse = mouseHandler,
		.onFinger = fingerHandler,
		.onDestroy = nOnDestroyFreeTarget,	
	});
	
	return self->base;
}

