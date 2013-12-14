
/*

	HamsiBro

*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "framework.h"

// General data

bool running;
SDL_Event event;

// Graphics

Texture *background;
Texture *hamsi1, *hamsi2, *hamsi3, *sleeping, *sleeping2, *walking, *walking2;
Texture *wheel;

// Sounds

Mix_Chunk *chord;

// --- Functions ---

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

	chord = Mix_LoadWAV ("res/f7chord.ogg");
}

void drawGameScene ()
{
	draw (background, 0, 0);
}

void gameScene ()
{
	running = true;
	
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
	gameScene ();
	cleanup ();
	
	return 0;
}

