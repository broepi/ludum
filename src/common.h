

#define FPS  30
#define FRAMEDUR  (1000.0/FPS)

#define idlingState 0
#define sleepingState 1
#define eatingState 2
#define walkingState 3
#define wheelingState 4
#define goDrinkingState 5
#define drinkingState 6
#define goEatingState 7
#define goWheelingState 8
#define deadState 9

#define drinkPointX 715
#define drinkPointY 496
#define eatPointX 721
#define eatPointY 324
#define wheelPointX 540
#define wheelPointY 275

#define walkSpeed 5

// hamsi state variable symbols
#define WATER 0
#define FOOD  1
#define POWER 2


// Classes .....................................................................

struct Hamsi {

	string name;
	unsigned int age;  // hamsi's uptime counted in frames (yes, in FRAMES!!!)
	
	float x, y;
	int state;
	int lastchange; // counts frames from last state change
	float food;  // 0..10
	float water; // 0..10
	float power; // 0..10
	float health; // 0..10
	
	// all states
	int dwell; // for how many frames will the hamsi bro do this action
	
	// walking states
	int destx, desty;
	float vx, vy;
	
	int anima; // animation variant
	int anictr; // counting frames for animation
};

struct PooBean {
	int x, y;
	int style; // 0..1
};

