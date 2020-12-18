#include "Rounded.h"
#include <math.h>

typedef struct Rounded {
	sElement* base;
	SDL_Texture* texture;
} Rounded;


static void draw(Rounded* self, const char* color, Uint32 radius) {		
	
	const sRect posOnCamera = nElemCalcPosOnCamera(self->base);
	const sRect* pos = nAppCalcDest(&posOnCamera, &(sRect){0, 0, 0, 0});

	SDL_SetRenderTarget(nAppSDLRenderer(), self->texture);	

	SDL_SetRenderDrawColor(nAppSDLRenderer(), 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(nAppSDLRenderer());
	
	//draw squares
	enum {RIGHT, LEFT, BOTTOM, TOP, CENTER};
	
	const sRect SQUARES[5] = {
		[RIGHT] = {0, radius, radius, pos->h - (2 * radius)},
		[LEFT] = {pos->w - radius, radius, radius, pos->h - (2 * radius)},
		[BOTTOM] = {radius, 0, pos->w - (2 * radius), radius},
		[TOP] = {radius, pos->h - radius, pos->w - (2 * radius), radius},
		[CENTER] = {radius, radius, pos->w - (2 * radius), pos->h - (2 * radius)}
	};

	SDL_Color* c = nAppConvertColor(&(SDL_Color){0, 0, 0, 0}, color);
	SDL_SetRenderDrawColor(nAppSDLRenderer(), c->r, c->g, c->b, c->a);
	SDL_RenderFillRects(nAppSDLRenderer(), SQUARES, 5);	
	
	
	//draw semicircles
	for (Uint32 i = 1; i <= radius; i++) {		
		bool inside = false;
		for (Uint32 j = radius; j >= i ; j--) {			
			double ratio = 1.0;
			
			if (!inside) {			
				ratio = sqrt(i * i + j * j);
				if (ratio <= (double) radius) {
					ratio = 1.0;
					inside = true;
				}
				else if (ratio >= (double) radius + 1.0) {
					ratio = 0.0;
				}
				else {
					ratio = (radius + 1.0) - ratio;
				}
			}

			SDL_SetRenderDrawColor(nAppSDLRenderer(), c->r, c->g, c->b, (Uint8) (c->a * ratio));
			
			int numberOfPoints = 4;

			sPoint points[8] = {
				[0] = {radius - j, radius - i},
				[1] = {pos->w - radius + j - 1, radius - i},
				[2] = {pos->w - radius + j - 1, pos->h - radius + i - 1},
				[3] = {radius - j, pos->h - radius + i - 1},
			};
						
			if (i != j) {
				numberOfPoints = 8;				
				points[4] = (sPoint) {radius - i, radius - j};
				points[5] = (sPoint) {pos->w - radius + i - 1, radius - j};
				points[6] = (sPoint) {pos->w - radius + i - 1, pos->h - radius + j - 1};
				points[7] = (sPoint) {radius - i, pos->h - radius + j - 1};				
			}
			SDL_RenderDrawPoints(nAppSDLRenderer(), points, numberOfPoints);
		}		
	}
	
	SDL_SetRenderDrawColor(nAppSDLRenderer(), 0, 0, 0, 255);
	SDL_SetRenderTarget(nAppSDLRenderer(), NULL);
}


static void onRender(sEvent* e) {
	Rounded* self = e->target;	
	const sRect posOnCamera = nElemCalcPosOnCamera(self->base);
	const sRect* posOnWindow = nAppCalcDest(&posOnCamera, &(sRect){0, 0, 0, 0});
	SDL_RenderCopy(nAppSDLRenderer(), self->texture, NULL, posOnWindow);	
}

static void onDestroy(sEvent* e) {
	Rounded* self = e->target;
	SDL_DestroyTexture(self->texture);
	free(self);
}


sElement* nRoundedCreate(sElement* base, const char* color, Uint32 radius) {
	
	nUtilAssertNullPointer(base);	
	const sRect* pos = nElemPosition(base);
	nUtilAssertArgument(pos);

	Rounded* self = nUtilAssertAlloc(malloc(sizeof(Rounded)));
	self->base = base;

	const sRect posOnCamera = nElemCalcPosOnCamera(self->base);
	const sRect* posOnWindow = nAppCalcDest(&posOnCamera, &(sRect){0, 0, 0, 0});
	
	self->texture = SDL_CreateTexture(nAppSDLRenderer(), SDL_PIXELFORMAT_RGBA8888, 
		SDL_TEXTUREACCESS_TARGET, posOnWindow->w, posOnWindow->h
	);

	SDL_SetTextureBlendMode(self->texture, SDL_BLENDMODE_BLEND);

	Uint32 temp = (Uint32)(posOnWindow->w * radius / (double) pos->w);
	temp = temp > posOnWindow->w / 2u ? posOnWindow->w /2u : temp;
	temp = temp > posOnWindow->h / 2u ? posOnWindow->h / 2u : temp;

	draw(self, color, temp);

	nElemAddComponent(self->base, &(sComponent){
		.name = "nRoundedBackground",
		.target = self,
		.onDestroy = onDestroy,
		.onRender = onRender,
	});

	return base;
}