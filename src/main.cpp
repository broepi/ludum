
/*

	HamsiBro

*/

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "framework.h"

using namespace std;

#define FPS  30
#define FRAMEDUR  (1000.0/FPS)

#define idlingState 0
#define sleepingState 1
#define eatingState 2
#define walkingState 3
#define wheelingState 4
#define goDrinkingState 4

// Classes .....................................................................

struct Hamsi {
	int x, y;
	int state;
	int lastchange; // counts frames from last state change
	float food;  // 0..100
	float water; // 0..100
	
	// walking state
	float vx, vy;
	
	int anima; // animation variant
	int anictr; // counting frames for animation
};

// General data -----------------------------------------------------------------------

bool running;
SDL_Event event;
int lasttick, curtick;

// Graphics -----------------------------------------------------------------------

Texture *background;
Texture *hamsi1, *hamsi2, *hamsi3, *sleeping, *sleeping2, *walking, *walking2;
Texture *wheel;
Texture *bottle;
Texture *foodPoint, *waterPoint, *foodPointE, *waterPointE;

// Sounds -----------------------------------------------------------------------

Mix_Chunk *chord;

// Objects -----------------------------------------------------------------------

Hamsi *hamsi;

// Game state variables

//bool sidebar

// --- Functions --- -----------------------------------------------------------------------

void loadResources ()
{
	background = loadTexture ("res/background.png");
	hamsi1 = loadTexture ("res/hamsi1.png");
	hamsi2 = loadTexture ("res/hamsi2.png");
	hamsi3 = loadTexture ("res/hamsi3.png");
	sleeping = loadTexture ("res/sleeping.png");
	sleeping2 = loadTexture ("res/sleeping2.png");
	walking = loadTexture ("res/walking.png");
	walking2 = loadTexture ("res/walking2.png");
	wheel = loadTexture ("res/wheel.png");
	bottle = loadTexture ("res/bottle.png");
	foodPoint = loadTexture ("res/foodpoint.png"); 
	foodPointE = loadTexture ("res/foodpointE.png"); 
	waterPoint = loadTexture ("res/waterpoint.png"); 
	waterPointE  = loadTexture ("res/waterpointE.png"); 

	chord = Mix_LoadWAV ("res/f7chord.ogg");
}

void hamsiEnterIdling ()
{
	cout << "enter idling" << endl;
	hamsi->state = idlingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiEnterWalking ()
{
	cout << "enter walking" << endl;
	hamsi->vx = rand()%8 - 4;
	hamsi->vy = rand()%2 - 1;
	hamsi->state = walkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiGoDrinking ()
{
	cout << "go drinking" << endl;
	hamsi->vx = rand()%8 - 4;
	hamsi->vy = rand()%2 - 1;
	hamsi->state = walkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void handleGameMoment ()
{
	switch (hamsi->state) {
	
	case idlingState:
	
		// actions
		if (hamsi->lastchange > FPS*1) {
			hamsiEnterWalking ();
		}
	
		hamsi->food -= 0.005 ;
		hamsi->water -= 0.01 ;
	
		// Animation
		if (hamsi->anima == 0 && rand()%50 == 1) {
			hamsi->anima = 1 + rand() % 2;
			hamsi->anictr = 0;
		}
		else if (hamsi->anima != 0 && hamsi->anictr > 5) {
			hamsi->anima = 0;
			hamsi->anictr = 0;
		}
		break;
		
	case walkingState:
	
		// actions
		if (hamsi->lastchange > FPS*1) {
			hamsiEnterIdling ();
		}
	
		hamsi->food -= 0.005 ;
		hamsi->water -= 0.01 ;
		
		hamsi->x += hamsi->vx;
		hamsi->y += hamsi->vy;
	
		// Animation
		if (hamsi->anictr > 2) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		break;
	}
	
	hamsi->anictr ++;
	hamsi->lastchange ++;
}

void drawHamsi ()
{
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
		bool hflip = hamsi->vx > 0;
		switch (hamsi->anima) {
		case 0:
			draw (walking, hamsi->x, hamsi->y, 32, 32, hflip);
			break;
		case 1:
			draw (walking2, hamsi->x, hamsi->y, 32, 32, hflip);
			break;
		}
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
	
}

void gameStage ()
{

	// Preparation
	hamsi = new Hamsi;
	hamsi->x = 400;
	hamsi->y = 400;
	hamsi->food = 100;
	hamsi->water = 100;
	
	hamsiEnterIdling ();
	
	
	running = true;
	
	lasttick = SDL_GetTicks ();
	while (running) {
		while (SDL_PollEvent (&event) == 1) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				Mix_PlayChannel (-1, chord, 0);
				break;
			}
		}
		
		curtick = SDL_GetTicks ();
		if (curtick-lasttick >= FRAMEDUR) {
			lasttick = curtick;
			
			handleGameMoment ();
			
			SDL_SetRenderDrawColor (renderer, 0, 0, 0xff, 0xff);
			SDL_RenderClear (renderer);
			
			drawGameScene ();
			
			SDL_RenderPresent (renderer);
		}
	}
}

int main (int argc, char *argv[])
{
	init ();
	loadResources ();
	gameStage ();
	cleanup ();
	
	return 0;
}

