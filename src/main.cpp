
/*

	HamsiBro

*/

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <list>
#include <string>
#include <sstream>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

using namespace std;

#include "SDL_ttf.h"

#include "common.h"
#include "framework.h"
#include "drawing.h"


void hamsiEnterIdling ();

// General data -----------------------------------------------------------------------

TTF_Font *font;

const SDL_Rect rcBtnWater = {736,64,64,64};
const SDL_Rect rcBtnFood = {736,128,64,64};
const SDL_Rect rcLink = {800-32,64,32,32};

bool running;
bool brokeUp = false;
SDL_Event event;
int lasttick, curtick;

// Sounds -----------------------------------------------------------------------

Mix_Chunk *chord;
Mix_Chunk *sndFillBowl, *sndFillBottle, *sndCrunch, *sndDrink, *sndSnip, *sndPlop;
Mix_Chunk *sndWalking, *sndWheeling;
Mix_Chunk *sndRIP;

// Objects -----------------------------------------------------------------------

Hamsi *hamsi;
list<PooBean*> pooBeans;
TextLabel *lblNamePrompt;
TextLabel *lblNameInput;
TextLabel *lblName1, *lblName2, *lblAge1, *lblAge2;
TextLabel *lblEnd1, *lblEnd2;

// Colors *****************************************************************************

SDL_Color white = {0xff, 0xff, 0xff, 0xff};
SDL_Color black = {0, 0, 0, 0xff};

// Game state variables

bool newGame = true;

bool sidebarOut;
float bottleWater; // 0..10
float bowlFood; // 0..10

/*
bool sndloopWalk = false;
bool sndloopWheel = false;
bool sndloopDrink = false;
bool sndloopEat = false;
*/
int sndchWalk, sndchWheel, sndchDrink, sndchEat;

// --- Functions --- -----------------------------------------------------------------------

void stopLoops ()
{
	Mix_HaltChannel (sndchWalk);
	Mix_HaltChannel (sndchWheel);
	Mix_HaltChannel (sndchDrink);
	Mix_HaltChannel (sndchEat);
}

void playLoop (Mix_Chunk *snd, int *chvar)
{
	stopLoops ();
	*chvar = Mix_PlayChannel (-1, snd, -1);
}

void createHamsi ()
{
	hamsi = new Hamsi;
	hamsi->name = "";
	hamsi->age = 0;
	hamsi->x = 400;
	hamsi->y = 400;
	hamsi->food = 6;
	hamsi->water = 6;
	hamsi->power = 6;
	hamsi->health = 6;
	
	hamsiEnterIdling ();
}

void saveToDisk ()
{
	FILE *fs = fopen ("savegame", "wb");
	
	fwrite (&newGame, sizeof (bool), 1, fs);
	
	fwrite (&hamsi->age, sizeof(Hamsi)-sizeof(string), 1, fs);
	
	const char *name = hamsi->name.c_str ();
	int namelen = strlen (name);
	fwrite (&namelen, sizeof(int), 1, fs);
	fwrite (name, sizeof(char), namelen, fs);
	
	int beancnt = pooBeans.size ();
	fwrite (&beancnt, sizeof(int), 1, fs);
	for (list<PooBean*>::iterator it = pooBeans.begin (); it != pooBeans.end (); it ++) {
		PooBean *bean = *it;
		fwrite (bean, sizeof (PooBean), 1, fs);
	}
	
	fwrite (&bottleWater, sizeof (float), 1, fs);
	fwrite (&bowlFood, sizeof (float), 1, fs);
	
	fclose (fs);
}

void loadFromDisk ()
{
	FILE *fs = fopen ("savegame", "rb");
	
	if (fs==0) {
		
		cout << "Did you try to delete the save file? This doesn't help!" << endl;
		exit (-1);
		
		createHamsi ();
		pooBeans.clear ();
		bottleWater = 10;
		bowlFood = 10;
		saveToDisk ();
		return;
	}
	
	fread (&newGame, sizeof(bool), 1, fs);
	if (newGame) {
		cout << "NEWGAME" << endl;
		createHamsi ();
		pooBeans.clear ();
		bottleWater = 10;
		bowlFood = 10;
	}
	else {
		hamsi = new Hamsi;
		fread (&hamsi->age, sizeof(Hamsi)-sizeof(string), 1, fs);
		int namelen;
		fread (&namelen, sizeof(int), 1, fs);
		char *name = new char [namelen+1];
		name [namelen] = 0;
		fread (name, sizeof(char), namelen, fs);
		hamsi->name = name;
		int beancnt;
		fread (&beancnt, sizeof(int), 1, fs);
		pooBeans.clear ();
		for (int i=0; i<beancnt; i++) {
			PooBean *newBean = new PooBean;
			fread (newBean, sizeof(PooBean), 1, fs);
			pooBeans.push_back (newBean);
		}
		fread (&bottleWater, sizeof (float), 1, fs);
		fread (&bowlFood, sizeof (float), 1, fs);
	}
	
	fclose (fs);
}

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
	sndWalking  = Mix_LoadWAV ("res/walk.ogg");
	sndWheeling  = Mix_LoadWAV ("res/wheeling.ogg");
	sndRIP  = Mix_LoadWAV ("res/gameover.ogg");
	
	font = TTF_OpenFont ("res/Ubuntu-R.ttf", 20);
	if (font == 0) {
		cerr << "TTF error: " << TTF_GetError () << endl;
		exit (-1);
	}
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

string humanReadableAge ()
{
	double seconds = hamsi->age / FPS;
	string result = "";
	stringstream ss;

	ss << (((int)seconds)%60) << " seconds";
	result += ss.str ();
	ss.str ("");
	if ( seconds > 60 ) {
		double minutes = seconds / 60;
		ss << (((int)minutes)%60) << " minutes ";
		result = ss.str () + result;
		ss.str ("");
		if ( minutes > 60 ) {
			double hours = minutes / 60;
			ss << " " << (((int)hours)%24) << " hours ";
			result = ss.str () + result;
			ss.str ("");
			if ( hours > 24 ) {
				double days = hours / 24;
				ss << " " << ((int)days) << " days ";
				result = ss.str () + result;
				ss.str ("");
			}
		}
	}
	
	return result;
}

void exhaustBottle ()
{
	bottleWater -= 1.0 / (6*FPS);
	bottleWater = max (bottleWater, 0.0f);
}

void exhaustBowl ()
{
	bowlFood -= 1.0 / (12*FPS);
	bowlFood = max (bowlFood, 0.0f);
}

void updateHamsiVars ()
{
	const float
		waterInc = 1.0 / ( 1*FPS),  foodInc = 1.0 / ( 1*FPS) , powerInc = 1.0 / (60*FPS),
		waterDec = 1.0 / (60*FPS),  foodDec = 1.0 / (120*FPS), powerDec = 1.0 / (360*FPS);
	/*
	const float
		waterInc = 1.0 / ( 1*FPS),  foodInc = 1.0 / ( 1*FPS) , powerInc = 1.0 / (60*FPS),
		waterDec = 1.0 / (6*FPS),  foodDec = 1.0 / (12*FPS), powerDec = 1.0 / (360*FPS);
	*/
		
	float healthChange;
	
	healthChange =
		(hamsi->water + hamsi->food + hamsi->power - 15.0 - pooBeans.size ()) / (30.0 * FPS);
	
	switch (hamsi->state) {
	
	case idlingState:
	case walkingState:
	case goDrinkingState:
	case goEatingState:
	case goWheelingState:
	case wheelingState:
	
		hamsi->water = max (hamsi->water - waterDec, 0.0f);
		hamsi->food = max (hamsi->food - foodDec, 0.0f);
		hamsi->power = max (hamsi->power - powerDec, 0.0f);
		break;
	
	case drinkingState:
	
		hamsi->water = min (hamsi->water + waterInc, 10.0f);
		hamsi->food = max (hamsi->food - foodDec, 0.0f);
		hamsi->power = max (hamsi->power - powerDec, 0.0f);
		break;

	case eatingState:
	
		hamsi->water = max (hamsi->water - waterDec, 0.0f);
		hamsi->food = min (hamsi->food + foodInc, 10.0f);
		hamsi->power = max (hamsi->power - powerDec, 0.0f);
		break;

	case sleepingState:
		
	
		hamsi->water = max (hamsi->water - waterDec, 0.0f);
		hamsi->food = max (hamsi->food - foodDec, 0.0f);
		hamsi->power = min (hamsi->power + powerInc, 10.0f);
		break;
	
	}
	
	hamsi->health = max (min (hamsi->health + healthChange, 10.0f), 0.0f);
}

void refreshBottle ()
{
	bottleWater = 10;
}

void refreshBowl ()
{
	bowlFood = 10;
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
	stopLoops ();
	hamsi->state = idlingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = randInt (FPS*1, FPS*16);
	cout << "idling for secs " << hamsi->dwell / (float)FPS << endl;
}

void hamsiEnterSleeping ()
{
	stopLoops ();
	hamsi->state = sleepingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	cout << "sleeping" << endl;
}

void hamsiEnterEating ()
{
	playLoop (sndCrunch, &sndchEat);
	hamsi->state = eatingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	cout << "eating" << endl;
}

void hamsiEnterWalking ()
{
	playLoop (sndWalking, &sndchWalk);
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
	playLoop (sndWheeling, &sndchWheel);
	hamsi->state = wheelingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	hamsi->dwell = randInt (FPS*1, FPS*16);
	cout << "wheeling for seconds " << hamsi->dwell / (float)FPS << endl;
}

void hamsiGoDrinking ()
{
	playLoop (sndWalking, &sndchWalk);
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
	playLoop (sndDrink, &sndchDrink);
	hamsi->state = drinkingState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	cout << "drinking" << endl;
}

void hamsiGoEating ()
{
	playLoop (sndWalking, &sndchWalk);
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
	playLoop (sndWalking, &sndchWalk);
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

void hamsiDie ()
{
	stopLoops ();
	playSnd (sndRIP);
	hamsi->state = deadState;
	hamsi->lastchange = 0;
	hamsi->anima = 0;
	hamsi->anictr = 0;
	
	cout << "hamster died" << endl;
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
	if (hamsi->food < minval && bowlFood > 0) {
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
	lblAge2->text = humanReadableAge ();
	lblAge2->update ();
	lblAge2->cx = lblAge2->w;
	lblAge2->cy = 0;
	
	if (hamsi->state != deadState && hamsi->health <= 0) {
		Mix_HaltChannel (-1);
		hamsiDie ();
	}

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
	
		updateHamsiVars ();
	
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
		else if ( checkForNeeds () ) {
			break;
		}
		
		if ( randInt (1,100) == 1 )
			poop ();
		
	
		updateHamsiVars ();
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

		updateHamsiVars ();

		
		
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
		
		updateHamsiVars ();
		exhaustBottle ();
				
		break;

	case eatingState:
	
		// actions / state transitions
		if (hamsi->food >= 10 || bowlFood <= 0) {
			hamsiEnterWalking ();
			break;
		}

		updateHamsiVars ();
		exhaustBowl ();
		
	
		// Animation
		if (hamsi->anictr > 3) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		
		break;

	case wheelingState:
	
		// actions / state transitions
		if (hamsi->lastchange > hamsi->dwell) {
			hamsi->y += 20;
			hamsiEnterWalking ();
			break;
		}
		else if ( checkForNeeds () ) {
			break;
		}
		
		updateHamsiVars ();
	
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
		
		updateHamsiVars ();
	
		// Animation
		if (hamsi->anictr > 4) {
			hamsi->anictr = 0;
			hamsi->anima = hamsi->anima == 1 ? 0 : 1;
		}
		
		break;

	}
	
	if (hamsi->state != deadState)
		hamsi->age ++;

	hamsi->anictr ++;
	hamsi->lastchange ++;
}


void birthStage ()
{
	// Preparation
	
	lblNamePrompt = new TextLabel (
		font, "How do you want to call your Hamsi Bro?", 400, 200, white);
	lblNameInput = new TextLabel (
		font, "", 400, 300, white);

	SDL_StartTextInput ();
	
	// Lé GAMELOOP
	running = true;
	brokeUp = false;
	
	lasttick = SDL_GetTicks ();
	while (running) {
	
		// EVENTS
		
		while (SDL_PollEvent (&event) == 1) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				brokeUp = true;
				break;
			case SDL_TEXTINPUT:
				lblNameInput->text += event.text.text;
				lblNameInput->update ();
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_BACKSPACE) {
					if (lblNameInput->text.size () > 0) {
						lblNameInput->text.erase (lblNameInput->text.size () -1);
						lblNameInput->update ();
					}
				}
				else if (event.key.keysym.sym == SDLK_RETURN &&
					lblNameInput->text != "" &&
					lblNameInput->text != " ")
				{
					cout << "Hamsi name: " << lblNameInput->text << endl;
					hamsi->name = lblNameInput->text;
					running = false;
				}
				break;
			}
		}
		
		// DRAWING
		
		curtick = SDL_GetTicks ();
		if (curtick-lasttick >= FRAMEDUR) {
			lasttick = curtick;
			
			SDL_SetRenderDrawColor (renderer, 0, 0, 0xff, 0xff);
			SDL_RenderClear (renderer);
			
			drawBirthScene ();
			
			SDL_RenderPresent (renderer);
		}
	}
	
	SDL_StopTextInput ();
}


void gameStage ()
{

	// Preparation
	
	lblName1 = new TextLabel (font, "Name:", 800-16, 200, white);
	lblName1->cx = lblName1->w;
	lblName1->cy = 0;
	lblName2 = new TextLabel (font, hamsi->name, 800-16, 220, white);
	lblName2->cx = lblName2->w;
	lblName2->cy = 0;
	lblAge1 = new TextLabel (font, "Age:", 800-16, 240, white);
	lblAge1->cx = lblAge1->w;
	lblAge1->cy = 0;
	lblAge2 = new TextLabel (font, "0", 800-16, 260, white);
	lblAge2->cx = lblAge2->w;
	lblAge2->cy = 0;
		
	sidebarOut = false;
	
	
	
	
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
					refreshBowl ();
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
					
					// else
					//hamsi->x = event.button.x;
					////hamsi->y = event.button.y;
				}
				break;
			case SDL_MOUSEMOTION:
				if (pointInBox (event.motion.x, event.motion.y, &rcLink))
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
			
			if (hamsi->state == deadState) {
				stringstream ss;
				ss << hamsi->name << " silently passed away.";
				lblEnd1 = new TextLabel (font, ss.str (), 400, 200, white);
				ss.str ("");
				ss << "He reached an age of " << humanReadableAge ();
				lblEnd2 = new TextLabel (font, ss.str (), 400, 230, white);
				ss.str ("");
			
				lblEnd1->draw ();
				lblEnd2->draw ();
				
				delete lblEnd1;
				delete lblEnd2;
			}			
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
	loadFromDisk ();
	if (newGame) {
		birthStage ();
		newGame = false;
	}
	if (!brokeUp)
		gameStage ();
	cleanup ();
	
	saveToDisk ();
	
	return 0;
}

