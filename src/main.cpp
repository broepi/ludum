
/*

	HamsiBro

*/

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

// General data -----------------------------------------------------------------------

bool running;
SDL_Event event;
int lasttick, curtick;

// Sounds -----------------------------------------------------------------------

Mix_Chunk *chord;

// Objects -----------------------------------------------------------------------

Hamsi *hamsi;

// Game state variables

bool sidebarOut;
int bottleWater; // 0..100
bool bowlFood;

// --- Functions --- -----------------------------------------------------------------------

void loadResources ()
{
	loadTextures ();

	chord = Mix_LoadWAV ("res/f7chord.ogg");
}

inline bool inReachOf (int x, int y, int ox, int oy)
{
	return ( x >= ox-10 && x <= ox+10 ) && ( y >= oy-10 && y <= oy+10 );
}

// State changes --------------------------------------------------------

void hamsiEnterIdling ()
{
	cout << "enter idling" << endl;
	hamsi->state = idlingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiEnterSleeping ()
{
	cout << "enter sleeping" << endl;
}

void hamsiEnterEating ()
{
	cout << "enter eating" << endl;
	hamsi->state = eatingState;
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

void hamsiEnterWheeling ()
{
	cout << "enter wheeling" << endl;
}

void hamsiGoDrinking ()
{
	cout << "go drinking" << endl;
	int dx = drinkPointX - hamsi->x;
	int dy = drinkPointY - hamsi->y;
	int d = sqrt ( dx*dx + dy*dy );
	hamsi->vx = (float)dx * 5 / (float) d;
	hamsi->vy = (float)dy * 5/ (float) d;
	hamsi->state = goDrinkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiEnterDrinking ()
{
	cout << "enter drinking" << endl;
	hamsi->state = drinkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiGoEating ()
{
	cout << "go eating" << endl;
	int dx = eatPointX - hamsi->x;
	int dy = eatPointY - hamsi->y;
	int d = sqrt ( dx*dx + dy*dy );
	hamsi->vx = (float)dx * 5 / (float) d;
	hamsi->vy = (float)dy * 5/ (float) d;
	hamsi->state = goEatingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

//////////////////////////////////////////////----------------------------------------------

void handleGameMoment ()
{
	switch (hamsi->state) {
	
	case idlingState:
	
		// actions
		if (hamsi->lastchange > FPS*1) {
			//hamsiEnterWalking ();
			hamsiGoDrinking ();
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
			hamsiGoEating ();
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
		
	case goDrinkingState:
	case goEatingState:
	
		// actions
		if (hamsi->state == goDrinkingState) {
			if (inReachOf (hamsi->x, hamsi->y, drinkPointX, drinkPointY)) {
				hamsiEnterDrinking ();
			}
		}
		else if (hamsi->state == goEatingState) {
			if (inReachOf (hamsi->x, hamsi->y, eatPointX, eatPointY)) {
				hamsiEnterEating ();
			}
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
	
	case drinkingState:
	
		// actions
		if (hamsi->lastchange > FPS*2) {
			hamsiGoEating ();
		}
		
		break;

	case eatingState:
	
		// actions
		if (hamsi->lastchange > FPS*2) {
			hamsiEnterIdling ();
		}
		
		break;

	}
	
	hamsi->anictr ++;
	hamsi->lastchange ++;
}


void gameStage ()
{

	// Preparation
	sidebarOut = false;
	bottleWater = 100;
	bowlFood = true;
	
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
			case SDL_MOUSEMOTION:
				if (!sidebarOut && event.motion.x > 800-16)
					sidebarOut = true;
				else if (sidebarOut && event.motion.x < 800-64)
					sidebarOut = false;
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

