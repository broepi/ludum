
/*

	HamsiBro

*/

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <list>
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
Mix_Chunk *sndFillBowl, *sndFillBottle, *sndCrunch, *sndDrink, *sndSnip, *sndPlop;

// Objects -----------------------------------------------------------------------

Hamsi *hamsi;
list<PooBean*> pooBeans;

// Game state variables

bool sidebarOut;
float bottleWater; // 0..10
bool bowlFood;

// --- Functions --- -----------------------------------------------------------------------

void loadResources ()
{
	loadTextures ();

	chord = Mix_LoadWAV ("res/f7chord.ogg");
	sndFillBowl = Mix_LoadWAV ("res/fillbowl.ogg");
	sndFillBottle = Mix_LoadWAV ("res/fillbottle.ogg");
	sndCrunch = Mix_LoadWAV ("res/crunch.ogg");
	sndDrink = Mix_LoadWAV ("res/drinking.ogg");
	sndSnip = Mix_LoadWAV ("res/snip.ogg");
	sndPlop = Mix_LoadWAV ("res/plop.ogg");
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
	hamsi->water -= 0.01;
	hamsi->water = max (hamsi->water, 0.0f);
}

void loadFood ()
{
	hamsi->food += 0.33;
	hamsi->food = min (hamsi->food, 10.0f);
}

void exhaustFood ()
{
	hamsi->food -= 0.005;
	hamsi->food = max (hamsi->food, 0.0f);
}

void loadPower ()
{
	hamsi->power += 0.1;
	hamsi->power = min (hamsi->power, 10.0f);
}

void exhaustPower ()
{
	hamsi->power -= 0.003;
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
}

void poop ()
{
	PooBean *newBean = new PooBean;
	newBean->x = hamsi->x;
	newBean->y = hamsi->y;
	newBean->style = randInt (0,1);
	pooBeans.push_back (newBean);
	playSnd (sndPlop);
}

// State changes --------------------------------------------------------

void hamsiEnterIdling ()
{
	hamsi->state = idlingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = randInt (FPS*1, FPS*16);
	cout << "idling for secs " << hamsi->dwell / (float)FPS << endl;
}

void hamsiEnterSleeping ()
{
	hamsi->state = sleepingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	cout << "sleeping" << endl;
}

void hamsiEnterEating ()
{
	hamsi->state = eatingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	playSnd (sndCrunch);
	cout << "eating" << endl;
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
	cout << "walking to " << hamsi->destx << " " << hamsi->desty << endl;
}

void hamsiEnterWheeling ()
{
	hamsi->state = wheelingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = randInt (FPS*1, FPS*16);
	cout << "wheeling for seconds " << hamsi->dwell / (float)FPS << endl;
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
	hamsi->state = drinkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	playSnd (sndDrink);
	cout << "drinking" << endl;
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

// ----- condensed state change

bool checkForNeeds ()
{
	int minval = 10;
	int minvar = 0; // hamsi state variable symbol

	if (hamsi->water < minval && bottleWater > 0) {
		minval = hamsi->water;
		minvar = WATER;
	}
	if (hamsi->food < minval && bowlFood) {
		minval = hamsi->food;
		minvar = FOOD;
	}
	if (hamsi->power < minval) {
		minval = hamsi->power;
		minvar = POWER;
	}
	
	if (minval < 5) {
		switch (minvar) {
		case WATER:
			hamsiGoDrinking ();
			return true;
		case FOOD:
			hamsiGoEating ();
			return true;
		case POWER:
			hamsiEnterSleeping ();
			return true;
		}
	}
	
	return false;
}

//////////////////////////////////////////////----------------------------------------------

void handleGameMoment ()
{
	switch (hamsi->state) {
	
	case idlingState:
	
		// actions / state transitions
		if (hamsi->lastchange > hamsi->dwell) {
			hamsiEnterWalking ();
			break;
		}
		else if ( checkForNeeds () )
			break;
		// randomly need for wheeling
		else if (randInt (1,100) == 1) {
			hamsiGoWheeling ();
			break;
		}
	
		exhaustWater ();
		exhaustFood ();
		exhaustPower ();
	
		// Animation
		if (hamsi->anima == 0 && randInt (1,50) == 1) {
			hamsi->anima = randInt (1,2);
			hamsi->anictr = 0;
		}
		else if (hamsi->anima != 0 && hamsi->anictr > 5) {
			hamsi->anima = 0;
			hamsi->anictr = 0;
		}
		break;
		
	case walkingState:
	
		// actions / state transitions
		if (hamsi->lastchange >= hamsi->dwell) {
			hamsi->x = hamsi->destx;
			hamsi->y = hamsi->desty;
			hamsiEnterIdling ();
			break;
		}
		else if ( checkForNeeds () )
			break;
		
		if ( randInt (1,100) == 1 )
			poop ();
		
	
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
	
		// actions / state transitions
		if (hamsi->lastchange >= hamsi->dwell) {
			hamsi->x = hamsi->destx;
			hamsi->y = hamsi->desty;
			switch (hamsi->state) {
				case goDrinkingState: hamsiEnterDrinking (); break;
				case goEatingState: hamsiEnterEating (); break;
				case goWheelingState: hamsiEnterWheeling (); break;
			}
			break;
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
	
		// actions / state transitions
		if (hamsi->water >= 10 || bottleWater <= 0) {
			hamsiEnterWalking ();
			break;
		}
		
		loadWater ();
		exhaustFood ();
		exhaustPower ();
		exhaustBottle ();
				
		break;

	case eatingState:
	
		// actions / state transitions
		if (hamsi->food >= 10) {
			bowlFood = false;
			hamsiEnterWalking ();
			break;
		}
		
		exhaustWater ();
		loadFood ();
		exhaustPower ();
		
		break;

	case wheelingState:
	
		// actions / state transitions
		if (hamsi->lastchange > hamsi->dwell) {
			hamsiEnterWalking ();
			break;
		}
		else if ( checkForNeeds () )
			break;
		
		exhaustWater ();
		exhaustFood ();
		exhaustPower ();
	
		// Animation
		if (hamsi->anictr > 1) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		
		break;

	case sleepingState:
	
		// actions / state transitions
		if (hamsi->power >= 10) {
			hamsiEnterWalking ();
		}
		
		exhaustWater ();
		exhaustFood ();
		loadPower ();
	
		// Animation
		if (hamsi->anictr > 4) {
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
	pooBeans.clear ();
	
	sidebarOut = false;
	bottleWater = 10;
	bowlFood = true;
	
	hamsi = new Hamsi;
	hamsi->x = 400;
	hamsi->y = 400;
	hamsi->food = 10;
	hamsi->water = 10;
	hamsi->power = 10;
	hamsi->health = 10;
	
	hamsiEnterIdling ();
	
	
	
	// Lé GAMELOOP
	
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
					playSnd (sndSnip);
					playSnd (sndFillBottle);
				}
				else if (pointInBox (event.button.x, event.button.y, &rcBtnFood)) {
					bowlFood = true;
					playSnd (sndSnip);
					playSnd (sndFillBowl);
				}
				else {
					// check if a bean was clicked
					for (list<PooBean*>::iterator it = pooBeans.begin ();
						it != pooBeans.end (); it ++)
					{
						SDL_Rect boundingBox = {(*it)->x - 16, (*it)->y - 16 , 32, 32};
						if (pointInBox (event.button.x, event.button.y, &boundingBox)) {
							pooBeans.erase (it);
							playSnd (sndSnip);
							break;
						}
					}
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
	cout << "Ah, okay! Someone's actually observing the standard output :-)" << endl;
	cout << "Welcome to Hamsi Bro !!!" << endl;
	cout << "Your one and only hamster brother. - Don't kill him, you won't get another one"<<endl;
	init ();
	loadResources ();
	gameStage ();
	cleanup ();
	
	return 0;
}

