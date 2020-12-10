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
	self->status &= (nButton->ON | nButton->HOVER);
	self->keyStatus &= nButton->ON;	
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
			self->status |= nButton->DOWN;
			self->status |= nButton->ON;
			self->clickFlag = true;
		}
	}

	else if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
		sPoint p = { e->button.x, e->button.y };		
		if (SDL_PointInRect(&p, &pos)) {	
			self->status |= nButton->UP;
			self->status &= ~nButton->ON;
			if (self->clickFlag) self->status |= nButton->CLICK;
			self->clickFlag = false;
		}
	}

	else if (e->type == SDL_MOUSEMOTION) {
		sPoint p = { e->motion.x, e->motion.y };
		if (SDL_PointInRect(&p, &pos)) {			
			self->status |= nButton->HOVER;
		}
		else {
			self->status &= ~(nButton->HOVER | nButton->ON);			
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
			self->status |= nButton->DOWN;
			self->status |= nButton->ON;
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
				self->status |= nButton->UP;	
				self->status &= ~nButton->ON;
				if (self->clickFlag){
					self->status |= nButton->CLICK;
				}
			}
			else {
				self->status &= ~(nButton->HOVER | nButton->ON);							
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
				self->status &= ~(nButton->HOVER | nButton->ON);
				self->clickFlag = false;
				setFingerId(self, NULL);			
			}			
		}
		else if(!self->fingerID){
			if(isInside){
				self->status |= nButton->HOVER;
				self->status |= nButton->ON;
				setFingerId(self, &e->tfinger.fingerId);
			}			
		}		
	}		
}

static void onKeyboard(sEvent* ev) {
	
	sButton* self = ev->target;
	const SDL_Event* e = ev->sdle;

	if (self->keyCode == e->key.keysym.sym) {
		if (e->type == SDL_KEYDOWN && !(self->keyStatus & nButton->ON)) {			
			self->keyStatus |= nButton->ON;
			self->keyStatus |= nButton->DOWN;				
		}
		else if (e->type == SDL_KEYUP) {
			self->keyStatus |= nButton->UP;
			self->keyStatus &= ~nButton->ON;
			self->keyStatus |= nButton->CLICK;
		}
	}
}


static Uint32 getStatus(sElement* base){
	sButton* self = nElemGetComponent(base, "nButton");
	Uint32 status = nButton->NONE;
	status |= self->input & nButton->SCREEN ? self->status : nButton->NONE;
	status |= self->input & nButton->KEYBOARD ? self->keyStatus : nButton->NONE;
	return status;
}

static bool hasStatus(sElement* base, Uint32 status) {
	sButton* self = nElemGetComponent(base, "nButton");
	return (
		((self->input & nButton->SCREEN) && (self->status & status)) ||
		((self->input & nButton->KEYBOARD) && (self->keyStatus & status))
	);
}

//constructor 
static void implement(sElement* base, Uint32 inputs, int keyCode) {	
	
	nUtil->assertArgument(!(inputs & nButton->SCREEN) || nElemPosition(base));

	sButton* self = nUtil->assertAlloc(calloc(1, sizeof(sButton)));
	self->input = inputs;
	self->keyCode = keyCode;
	self->base = base;

	sHandler keyboardHandler = (inputs & nButton->KEYBOARD) ? onKeyboard : NULL;
	sHandler mouseHandler = (inputs & nButton->MOUSE) ? onMouse : NULL;
	sHandler fingerHandler = (inputs & nButton->MOUSE) ? onFinger : NULL;
	#if 1
	nElemAddComponent(base, &(sComponent){
		.name = "nButton",
		.target = self,
		.onLoopBegin = onLoopBegin,
		.onKeyboard = keyboardHandler,
		.onMouse = mouseHandler,
		.onFinger = fingerHandler,
		.onDestroy = nComponent->onDestroyFreeTarget,	
	});	
	#endif
}

const struct sButtonNamespace* const nButton= &(struct sButtonNamespace) {
	.implement = implement,
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