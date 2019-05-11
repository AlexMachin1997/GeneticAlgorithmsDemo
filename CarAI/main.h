#ifndef GAME_PLAYERS

#define GAME_PLAYERS

//AI Variables
const int numOfInst = 1000;			// Starting number of instructions
int instructions[numOfInst];		// Instructions then gets assigned to the instructions variable
double MUTATION_RATE = 0.005;		// The frequency of new mutations in a single genome
const int chromoLength = numOfInst;	//Chrome length is equal to the number of instructions which is 300
const int playerPopulation = 20;	// Number of players allowed on the screen, in this example it's 20
bool gameComplete = false;			// Game success by default is false
int generationCount = 0;			// Generation count
int currentInstruction = 0;			//Current instruction is set to 0;
bool touchedGoal = false;			//If goal is touched
bool touchedWall = false;			//If wall is touched

class Goal
{
public:
	float x;
	float y;
	float width;
	float height;

	Goal();
	Goal(int nx, int ny, int nwdith, int nheight);
};

class Vehicle
{
public:
	const static int numOfInst = chromoLength; // number of instructions are set to the chromo length
	float instr[numOfInst];
	int currentInstruction = 0; //Current instruction number
	int distToGoal = 0; //Distance to goal
	double fitness = 0; //Fitness of AI
	double rankFitness = 0; // Rank of fitness
	float x = 0; //Starting point for X axis
	float y = 0; //Starting point for Y axis
	float px = 15; //Lines for rotation
	float py = 0; //Lines for rotation
	float playerRot = 0; //Player rotation
	float width = 0; //Width of the vehicle
	float height = 0; //Height of the vehicle

	bool wallHit = false;

	Vehicle();
	Vehicle(float nx, float ny, float nwidth, float nheight);

	int operator == (const Vehicle &p1)
	{
		return instr == p1.instr;
	}
};

class Wall
{
public:
	float x, y;
	float width, height;

	Wall();
	Wall(int newX, int newY, int newWidth, int newHeight);
};

#endif // !GAME_PLAYERS

