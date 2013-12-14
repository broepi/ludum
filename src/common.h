

#define FPS  30
#define FRAMEDUR  (1000.0/FPS)

#define idlingState 0
#define sleepingState 1
#define eatingState 2
#define walkingState 3
#define wheelingState 4
#define goDrinkingState 5
#define drinkingState 6

#define drinkPointX 715
#define drinkPointY 496


// Classes .....................................................................

struct Hamsi {
	float x, y;
	int state;
	int lastchange; // counts frames from last state change
	float food;  // 0..100
	float water; // 0..100
	
	// walking state
	float vx, vy;
	
	int anima; // animation variant
	int anictr; // counting frames for animation
};

