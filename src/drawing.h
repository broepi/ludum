

#define layerTree 0
#define layerHamsi 1
#define layerWheel 2
#define layerBowl 3
#define layerBottle 4
#define layerPooBean 5

void loadTextures ();

void drawBirthScene ();
void drawGameScene ();

typedef void (*DrawFunc) (PooBean*);

struct DrawingLayer {
	DrawingLayer (int _type, PooBean *_bean, int _x, int _y, int _cx, int _cy, DrawFunc _drawFunc);
	int type; // layer type
	PooBean *bean;  // for multiple poobeans
	int x, y;
	int cx, cy;
	DrawFunc drawFunc;
};
