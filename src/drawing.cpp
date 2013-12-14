
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "common.h"
#include "framework.h"
#include "drawing.h"

using namespace std;

// HAMSI

extern Hamsi *hamsi;
extern bool sidebarOut;
extern float bottleWater;
extern bool bowlFood;

// Graphics -----------------------------------------------------------------------

Texture *background;
Texture *hamsi1, *hamsi2, *hamsi3, *sleeping, *sleeping2, *walking, *walking2;
Texture *drinking;
Texture *omnomnom;
Texture *wheel;
Texture *bottleE, *bottleF;
Texture *bowl, *bowl2;
Texture *foodPoint, *waterPoint, *foodPointE, *waterPointE;
Texture *btnFood, *btnWater;

// Functions ----------------------------------------------------------------------

void loadTextures ()
{
	background = loadTexture ("res/background.png");
	hamsi1 = loadTexture ("res/hamsi1.png");
	hamsi2 = loadTexture ("res/hamsi2.png");
	hamsi3 = loadTexture ("res/hamsi3.png");
	sleeping = loadTexture ("res/sleeping.png");
	sleeping2 = loadTexture ("res/sleeping2.png");
	walking = loadTexture ("res/walking.png");
	walking2 = loadTexture ("res/walking2.png");
	drinking = loadTexture ("res/drinking.png");
	omnomnom = loadTexture ("res/omnomnom.png");
	wheel = loadTexture ("res/wheel.png");
	bottleE = loadTexture ("res/bottle.png");
	bottleF = loadTexture ("res/bottleF.png");
	bowl = loadTexture ("res/bowl.png");
	bowl2 = loadTexture ("res/bowl2.png");
	foodPoint = loadTexture ("res/foodpoint.png"); 
	foodPointE = loadTexture ("res/foodpointE.png"); 
	waterPoint = loadTexture ("res/waterpoint.png"); 
	waterPointE  = loadTexture ("res/waterpointE.png");
	btnFood = loadTexture ("res/btnFood.png");
	btnWater = loadTexture ("res/btnWater.png");
}


void drawHamsi ()
{
	bool hflip;
	switch (hamsi->state) {
	case idlingState:
		switch (hamsi->anima) {
		case 0:
			draw (hamsi1, hamsi->x, hamsi->y, 32, 32);
			break;
		case 1:
			draw (hamsi2, hamsi->x, hamsi->y, 32, 32);
			break;
		case 2:
			draw (hamsi3, hamsi->x, hamsi->y, 32, 32);
			break;
		}
		break;
	case walkingState:
	case goDrinkingState:
	case goEatingState:
		hflip = hamsi->vx > 0;
		switch (hamsi->anima) {
		case 0:
			draw (walking, hamsi->x, hamsi->y, 32, 32, hflip);
			break;
		case 1:
			draw (walking2, hamsi->x, hamsi->y, 32, 32, hflip);
			break;
		}
		break;
	case drinkingState:
		draw (drinking, hamsi->x, hamsi->y, 32, 32);
		break;
	case eatingState:
		draw (omnomnom, hamsi->x, hamsi->y, 32, 32);
		break;
	};
}

void drawGameScene ()
{
	Texture *tex;

	// background
	draw (background, 0, 0);
	
	// bowl
	if (bowlFood)
		draw (bowl2, 720, 350, 63, 57);
	else
		draw (bowl, 720, 350, 63, 57);

	// bottle
	//SDL_Rect srcrect;
	//SDL_Rect dstrect;
	int x = 750;
	int y = 500;
	int cx = 32;
	int cy = 125;
	float a = 1.0 - bottleWater / 10.0;
	float b = bottleWater / 10.0;
	const int waterbarTotal = 100 - 11;
	SDL_Rect srcrect = {0, 0, bottleE->w, 11 + waterbarTotal * a};
	SDL_Rect dstrect = {x-cx, y-cy, bottleE->w, 11 + waterbarTotal * a};
	SDL_Rect srcrectF = {0, 11 + waterbarTotal * a, bottleE->w, waterbarTotal * b + 28};
	SDL_Rect dstrectF = {x-cx, y-cy + 11 + waterbarTotal * a, bottleE->w, waterbarTotal * b + 28};
	SDL_RenderCopyEx (renderer, bottleE->tex, &srcrect, &dstrect, 0, 0, SDL_FLIP_NONE);
	SDL_RenderCopyEx (renderer, bottleF->tex, &srcrectF, &dstrectF, 0, 0, SDL_FLIP_NONE);
	
	// hamsi
	drawHamsi ();
	
	// status bars
	for (int i = 0; i<10; i++) {
		// food
		if (i < hamsi->food)
			tex = foodPoint;
		else
			tex = foodPointE;
		draw (tex, i*28 + 16, 600 - 16, 0, 32);
		// water
		if (i < hamsi->water)
			tex = waterPoint;
		else
			tex = waterPointE;
		draw (tex, 800 - i*28 - 16, 600 - 16, 32, 32);
	}
	
	// sidebar
	if (sidebarOut) {
		draw (btnWater, 800,0, 64,0);
		draw (btnFood, 800,64, 64,0);
	}
	
}
