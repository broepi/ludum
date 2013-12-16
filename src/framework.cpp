
/*

	HamsiBro

*/

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

using namespace std;

#include "SDL_ttf.h"

#include "framework.h"

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
	
	if (TTF_Init () != 0) {
		cerr << "TTF error: " << TTF_GetError () << endl;
		exit (-1);
	}
	
	Mix_Init (MIX_INIT_OGG);

	window = SDL_CreateWindow ("Hamsi Bro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, 0);

	renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
	
	
	Mix_OpenAudio (44100, AUDIO_S16SYS, 2, 1024);
	
	srand (time (0));
}

void cleanup ()
{
	Mix_CloseAudio ();
	
	Mix_Quit ();
	
	TTF_Quit ();
	
	SDL_DestroyRenderer (renderer);
	
	SDL_DestroyWindow (window);
	
	SDL_Quit ();
}

Texture *loadTexture (const char *filename)
{
	Texture *result = new Texture;
	result->tex = IMG_LoadTexture (renderer, filename);
	SDL_QueryTexture (result->tex, 0, 0, &result->w, &result->h);
	
	return result;
}

void draw (Texture *tex, int x, int y, int cx, int cy, bool hflip)
{
	SDL_Rect srcrect = {0, 0, tex->w, tex->h};
	SDL_Rect dstrect = {x-cx, y-cy, tex->w, tex->h};
	SDL_RenderCopyEx (renderer, tex->tex, &srcrect, &dstrect, 0, 0,
		hflip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void playSnd (Mix_Chunk *snd)
{
	Mix_PlayChannel (-1, snd, 0);
}


TextLabel::TextLabel (TTF_Font *_font, string _text, int _x, int _y, SDL_Color _color)
{
	font = _font; text = _text; x = _x; y = _y; color = _color;
	
	if (text == "")
		text = " ";
	
	SDL_Surface *surf = TTF_RenderText_Solid (font, text.c_str (), color);
	SDL_Texture *sdltex = SDL_CreateTextureFromSurface (renderer, surf);
	tex = new Texture;
	tex->tex = sdltex;
	w = tex->w = surf->w;
	h = tex->h = surf->h;
	cx = w/2;
	cy = h/2;
	SDL_FreeSurface (surf);
}

TextLabel::~TextLabel ()
{
	SDL_DestroyTexture (tex->tex);
	delete tex;
}

void TextLabel::draw ()
{
	::draw (tex, x, y, cx, cy);
}

void TextLabel::update ()
{
	SDL_DestroyTexture (tex->tex);
	delete tex;
	
	if (text == "")
		text = " ";
	
	SDL_Surface *surf = TTF_RenderText_Solid (font, text.c_str (), color);
	SDL_Texture *sdltex = SDL_CreateTextureFromSurface (renderer, surf);
	tex = new Texture;
	tex->tex = sdltex;
	w = tex->w = surf->w;
	h = tex->h = surf->h;
	cx = w/2;
	cy = h/2;
	SDL_FreeSurface (surf);
}




