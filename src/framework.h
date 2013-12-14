
struct Texture {
	SDL_Texture *tex;
	int w, h;
};


void init ();
void cleanup ();
Texture *loadTexture (char *filename);
void draw (Texture *tex, int x, int y, int cx = 0, int cy = 0, bool hflip = false);


extern SDL_Window *window;
extern SDL_Renderer *renderer;
