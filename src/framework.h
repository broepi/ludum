
struct Texture {
	SDL_Texture *tex;
	int w, h;
};

void init ();
void cleanup ();
Texture *loadTexture (char *filename);
void draw (Texture *tex, int x, int y);

extern SDL_Window *window;
extern SDL_Renderer *renderer;
