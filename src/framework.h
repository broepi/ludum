
struct Texture {
	SDL_Texture *tex;
	int w, h;
};

struct TextLabel {

	TextLabel (TTF_Font *font, string text, int x, int y, SDL_Color color);
	~TextLabel ();
	void draw ();
	void update ();
	
	TTF_Font *font;
	int x, y;
	int cx, cy;
	int w, h;
	string text;
	SDL_Color color;
	Texture *tex;
};


void init ();
void cleanup ();
Texture *loadTexture (char *filename);
void draw (Texture *tex, int x, int y, int cx = 0, int cy = 0, bool hflip = false);
void playSnd (Mix_Chunk *snd);


extern SDL_Window *window;
extern SDL_Renderer *renderer;
