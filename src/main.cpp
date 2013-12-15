
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

const SDL_Rect rcBtnWater = {736,0,64,64};
const SDL_Rect rcBtnFood = {736,64,64,64};

bool running;
SDL_Event event;
int lasttick, curtick;

// Sounds -----------------------------------------------------------------------

Mix_Chunk *chord;

// Objects -----------------------------------------------------------------------

Hamsi *hamsi;

// Game state variables

bool sidebarOut;
float bottleWater; // 0..10
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

inline bool pointInBox (int x, int y, const SDL_Rect *rect)
{
	return x >= rect->x && x < rect->x+rect->w && y >= rect->y && y < rect->y+rect->h;
}

inline int randInt (int first, int last)
{
	return rand () % (last + 1 - first) + first;
}

void loadWater ()
{
	hamsi->water += 0.33;
	hamsi->water = min (hamsi->water, 10.0f);
}

void exhaustWater ()
{
	hamsi->water -= 0.033;
	hamsi->water = max (hamsi->water, 0.0f);
}

void loadFood ()
{
	hamsi->food += 0.33;
	hamsi->food = min (hamsi->food, 10.0f);
}

void exhaustFood ()
{
	hamsi->food -= 0.033;
	hamsi->food = max (hamsi->food, 0.0f);
}

void loadPower ()
{
	hamsi->power += 0.33;
	hamsi->power = min (hamsi->power, 10.0f);
}

void exhaustPower ()
{
	hamsi->power -= 0.033;
	hamsi->power = max (hamsi->power, 0.0f);
}

void exhaustBottle ()
{
	bottleWater -= 0.05;
	bottleWater = max (bottleWater, 0.0f);
}

void refreshBottle ()
{
	bottleWater = 10;
}

void moveHamsi ()
{
	hamsi->x += hamsi->vx;
	hamsi->y += hamsi->vy;
	/*
	hamsi->x = min ((float)800-32, max ((float)0  +32, hamsi->x));
	hamsi->y = min ((float)600-32, max ((float)300+32, hamsi->y));
	*/
}

// State changes --------------------------------------------------------

void hamsiEnterIdling ()
{
	hamsi->state = idlingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = randInt (FPS*1, FPS*16);
	cout << "enter idling for secs " << hamsi->dwell / (float)FPS << endl;
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
	hamsi->destx = randInt (32, 800-32);
	hamsi->desty = randInt (300, 600-32);
	float dx = hamsi->destx - hamsi->x;
	float dy = hamsi->desty - hamsi->y;
	float dist = sqrt (dx*dx + dy*dy);
	hamsi->vx = dx * walkSpeed / dist;
	hamsi->vy = dy * walkSpeed / dist;
	hamsi->state = walkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = dist / walkSpeed;
	cout << "enter walking to " << hamsi->destx << " " << hamsi->desty << endl;
}

void hamsiEnterWheeling ()
{
	cout << "enter wheeling" << endl;
	hamsi->state = wheelingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
}

void hamsiGoDrinking ()
{
	hamsi->destx = drinkPointX;
	hamsi->desty = drinkPointY;
	float dx = hamsi->destx - hamsi->x;
	float dy = hamsi->desty - hamsi->y;
	float dist = sqrt (dx*dx + dy*dy);
	hamsi->vx = dx * walkSpeed / dist;
	hamsi->vy = dy * walkSpeed / dist;
	hamsi->state = goDrinkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = dist / walkSpeed;
	cout << "go drinking" << endl;
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
	hamsi->destx = eatPointX;
	hamsi->desty = eatPointY;
	float dx = hamsi->destx - hamsi->x;
	float dy = hamsi->desty - hamsi->y;
	float dist = sqrt (dx*dx + dy*dy);
	hamsi->vx = dx * walkSpeed / dist;
	hamsi->vy = dy * walkSpeed / dist;
	hamsi->state = goEatingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = dist / walkSpeed;
	cout << "go eating" << endl;
}

void hamsiGoWheeling ()
{
	hamsi->destx = wheelPointX;
	hamsi->desty = wheelPointY;
	float dx = hamsi->destx - hamsi->x;
	float dy = hamsi->desty - hamsi->y;
	float dist = sqrt (dx*dx + dy*dy);
	hamsi->vx = dx * walkSpeed / dist;
	hamsi->vy = dy * walkSpeed / dist;
	hamsi->state = goWheelingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = dist / walkSpeed;
	cout << "go wheeling" << endl;
}

//////////////////////////////////////////////----------------------------------------------

void handleGameMoment ()
{
	switch (hamsi->state) {
	
	case idlingState:
	
		// actions
		if (hamsi->lastchange > hamsi->dwell) {
			hamsiEnterWalking ();
		}
		else if (hamsi->power < 5) {
			hamsiGoWheeling ();
		}
		else if (hamsi->food < 5 && bowlFood) {
			hamsiGoEating ();
		}
		else if (hamsi->water < 5 && bottleWater > 0) {
			hamsiGoDrinking ();
		}
	
		exhaustWater ();
		exhaustFood ();
		exhaustPower ();
	
		// Animation
		if (hamsi->anima == 0 && rand()%50 == 1) {
			hamsi->anima = randInt (1,2);
			hamsi->anictr = 0;
		}
		else if (hamsi->anima != 0 && hamsi->anictr > 5) {
			hamsi->anima = 0;
			hamsi->anictr = 0;
		}
		break;
		
	case walkingState:
	
		// actions
		if (hamsi->lastchange >= hamsi->dwell) {
			hamsi->x = hamsi->destx;
			hamsi->y = hamsi->desty;
			hamsiEnterIdling ();
			//hamsiGoEating ();
		}
		
	
		exhaustWater ();
		exhaustFood ();
		exhaustPower ();

		
		moveHamsi ();
	
		// Animation
		if (hamsi->anictr > 2) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		break;
		
	case goDrinkingState:
	case goEatingState:
	case goWheelingState:
	
		// actions
		if (hamsi->state == goDrinkingState) {
			if (hamsi->lastchange >= hamsi->dwell) {
				hamsi->x = hamsi->destx;
				hamsi->y = hamsi->desty;
				hamsiEnterDrinking ();
			}
		}
		else if (hamsi->state == goEatingState) {
			if (hamsi->lastchange >= hamsi->dwell) {
				hamsi->x = hamsi->destx;
				hamsi->y = hamsi->desty;
				hamsiEnterEating ();
			}
		}
		else if (hamsi->state == goWheelingState) {
			if (hamsi->lastchange >= hamsi->dwell) {
				hamsi->x = hamsi->destx;
				hamsi->y = hamsi->desty;
				hamsiEnterWheeling ();
			}
		}
	
		exhaustWater ();
		exhaustFood ();
		exhaustPower ();
		
		
		moveHamsi ();
	
		// Animation
		if (hamsi->anictr > 2) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		break;
	
	case drinkingState:
	
		// actions
		if (hamsi->lastchange > FPS*2) {
			hamsiEnterWalking ();
		}
		
		loadWater ();
		exhaustFood ();
		exhaustPower ();
		exhaustBottle ();
		
		cout << "water fuelness: " << bottleWater << endl;
		
		break;

	case eatingState:
	
		// actions
		if (hamsi->lastchange > FPS*2) {
			bowlFood = false;
			hamsiEnterWalking ();
		}
		
		exhaustWater ();
		loadFood ();
		exhaustPower ();
		
		break;

	case wheelingState:
	
		// actions
		if (hamsi->lastchange > FPS*2) {
			bowlFood = false;
			hamsiEnterWalking ();
		}
		
		exhaustWater ();
		exhaustFood ();
		loadPower ();
	
		// Animation
		if (hamsi->anictr > 1) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
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
	bottleWater = 10;
	bowlFood = true;
	
	hamsi = new Hamsi;
	hamsi->x = 400;
	hamsi->y = 400;
	hamsi->food = 10;
	hamsi->water = 10;
	hamsi->power = 10;
	
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
				if (pointInBox (event.button.x, event.button.y, &rcBtnWater)) {
					refreshBottle ();
					Mix_PlayChannel (-1, chord, 0);
				}
				else if (pointInBox (event.button.x, event.button.y, &rcBtnFood)) {
					bowlFood = true;
					Mix_PlayChannel (-1, chord, 0);
				}
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

