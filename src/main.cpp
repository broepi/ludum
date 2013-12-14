
#include <SDL.h>
#include <SDL_mixer.h>

SDL_Window *window;
SDL_Renderer *renderer;
bool running;
SDL_Event event;
Mix_Chunk *chord;

int main (int argc, char *argv[])
{
	SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO);

	window = SDL_CreateWindow ("broepis game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, 0);

	renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
	
	Mix_Init (MIX_INIT_OGG);
	
	Mix_OpenAudio (44100, AUDIO_S16SYS, 2, 1024);
	
	chord = Mix_LoadWAV ("res/f7chord.ogg");
	
	SDL_SetRenderDrawColor (renderer, 0, 0, 0xff, 0xff);
	
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
			SDL_RenderClear (renderer);
			SDL_RenderPresent (renderer);
		}
	}
	
	Mix_FreeChunk (chord);
	
	Mix_CloseAudio ();
	
	Mix_Quit ();
	
	SDL_DestroyRenderer (renderer);
	
	SDL_DestroyWindow (window);
	
	SDL_Quit ();
	
	return 0;
}

