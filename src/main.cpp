
#include <SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;
bool running;
SDL_Event event;

int main (int argc, char *argv[])
{
	SDL_Init (SDL_INIT_VIDEO);

	window = SDL_CreateWindow ("broepis game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, 0);

	renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
	
	SDL_SetRenderDrawColor (renderer, 0, 0, 0xff, 0xff);
	
	running = true;
	
	while (running) {
		while (SDL_PollEvent (&event) == 1) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
			SDL_RenderClear (renderer);
			SDL_RenderPresent (renderer);
		}
	}
	
	SDL_DestroyRenderer (renderer);
	
	SDL_DestroyWindow (window);
	
	SDL_Quit ();
	
	return 0;
}

