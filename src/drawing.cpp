
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

// Graphics -----------------------------------------------------------------------

Texture *background;
Texture *hamsi1, *hamsi2, *hamsi3, *sleeping, *sleeping2, *walking, *walking2;
Texture *drinking;
Texture *wheel;
Texture *bottle;
Texture *bowl;
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
	wheel = loadTexture ("res/wheel.png");
	bottle = loadTexture ("res/bottle.png");
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
		draw (drinking, hamsi->x, hamsi->y, 32, 32, hflip);
		break;
	};
}

void drawGameScene ()
{
	Texture *tex;

	draw (background, 0, 0);
	
	drawHamsi ();

	draw (bottle, 750, 500, 32, 125);
	
	// status bars
	for (int i = 0; i<10; i++) {
		// food
		if (i*10 < hamsi->food)
			tex = foodPoint;
		else
			tex = foodPointE;
		draw (tex, i*28 + 16, 600 - 16, 0, 32);
		// water
		if (i*10 < hamsi->water)
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
