
/*

	HamsiBro

*/

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "framework.h"

using namespace std;

// General data

SDL_Window *window;
SDL_Renderer *renderer;

// --- Functions ---

void init ()
{	
	if (SDL_Init (SDL_INIT_VIDEO) != 0) {
		cerr << "SDL error: " << SDL_GetError () << endl;
		exit (-1);
	}
	
	if (IMG_Init (IMG_INIT_PNG) == 0) {
		cerr << "IMG error: " << IMG_GetError () << endl;
		exit (-1);
	}
	
	Mix_Init (MIX_INIT_OGG);

	window = SDL_CreateWindow ("broepis game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, 0);

	renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
	
	
	Mix_OpenAudio (44100, AUDIO_S16SYS, 2, 1024);
}

void cleanup ()
{
	Mix_CloseAudio ();
	
	Mix_Quit ();
	
	SDL_DestroyRenderer (renderer);
	
	SDL_DestroyWindow (window);
	
	SDL_Quit ();
}

Texture *loadTexture (char *filename)
{
	Texture *result = new Texture;
	result->tex = IMG_LoadTexture (renderer, filename);
	SDL_QueryTexture (result->tex, 0, 0, &result->w, &result->h);
	
	return result;
}

void draw (Texture *tex, int x, int y)
{
	SDL_Rect srcrect = {0, 0, tex->w, tex->h};
	SDL_Rect dstrect = {x, y, tex->w, tex->h};
	//SDL_SetTextureAlphaMod (tex->tex, 255);
	SDL_RenderCopyEx (renderer, tex->tex, &srcrect, &dstrect, 0, 0, SDL_FLIP_NONE);
}

