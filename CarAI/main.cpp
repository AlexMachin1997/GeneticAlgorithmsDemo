//-----------------------------------------------------------------------
//  
//------------------------------------------------------------------------
//#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <time.h>
#include <SDKDDKVer.h>

// Windows Header Files:

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <math.h>
#include <sstream>
#include <string>


#include <list>
#include <iostream>

using namespace std;

// MACROS
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) //Macro 1
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1) //Macro 2

#define WINDOW_WIDTH  600 //Window width
#define WINDOW_HEIGHT 400 // Window height

//--------------------------------- Globals ------------------------------
//
//------------------------------------------------------------------------

//Forward Declaration of functions
void initGame();
void updateGame();
void drawGame();
void drawGame(HDC &hdc);
void drawFrame(HDC &hdc, HWND);
void getKeys();
void getAIPopulation();
void sortPopulation();

#define CROSSOVER_RATE	0.7 //Porbability of a crossover happening

//Variables

//Coordinates
float playerx = WINDOW_WIDTH/2; //Where player will be placed on the X axis, so in this example its half of the window 
float playery = WINDOW_HEIGHT/2; //Where player will be placed on the Y axis, so in this example its half the window

//Rotation
float px = 15; 
float py = 0; 
float playerRot = 0; // Player rotation

const int numOfInst = 500;			// Starting number of instructions
int instructions[numOfInst];		// Instructions then gets assigned to the instructions variable
int instPtr = 0;					// Instructionm pointer, used in getAIPopulation to identify what number of instruction is currently selected e.g. 300 or 400

double MUTATION_RATE = 0.005;		 // The frequency of new mutations in a single genome
const int chromoLength = numOfInst;	 //Chrome length is equal to the number of instructions which is 300
const int population = 20;				// Number of players allowed on the screen, in this example it's 20
bool gameComplete = false;					//Game success by default is false
int generationCount = 0;						 // Generation count


//bool success = true; // Not sure if this is needed or not. gameComplete performs the same thing ?????
int currentInstruction = 0;


class Goal {
public: 

	float x;
	float y;
	float width;
	float height;

	Goal()	{
		x = 0; 
		y = 0;
		width = 0;
		height = 0;
	}

	Goal(int nx, int ny, int nwidth, int nheight) {
		x = nx;
		y = ny;
		width = nwidth; 
		height = nheight;
	}
};

//gameGoal created with class Goal
Goal gameGoal(400, 200, 100, 100); // X and Y & Height and Width 

class Vehicle{
	public:
	 
		const static int numOfInst = chromoLength;
		float instr[numOfInst];
		int currentInstruction = 0; //Current instruction number
		int distToGoal = 0; //Distance to goal
		double fitness = 0; //Fitness of AI
		double rankFitness = 0;
		float x = 5; 
		float y = 5;
		float px = 15;
		float py = 0;
		float playerRot = 0; //Player rotation
		float width; //Width of the vehicle
		float height; //Height of the vehicle

		Vehicle() {

			x = 0; y = 0;
			width = 0; height = 0;
			for (int i = 0; i < numOfInst; i++) {
				//instr[i] = 0; //born with the instinct to move right
				//instr[i] = 3; //SLoth Mode - born to sit still
				instr[i] = (rand() % 4); //random genome
			}
			fitness = 0;
		}

		//Constructor
		Vehicle(float nx, float ny, float nwidth, float nheight)
		{
			x = nx;
			y = ny;
			width = nwidth;
			height = nheight;
		}

		int operator == (const Vehicle &p1)
		{
			return instr == p1.instr;
		}
};

Vehicle players[population]; //Sets the population of the Vehicle class e.g. 20 or 30

int generation; //Declaration for generation number
float totalFitness; // Declaration of totalFitness

void evolve(); //Forward declaration for evolve function
void decode(Vehicle); // Not sure if this is needed, the function is currently blank

Vehicle& selection();
Vehicle& tournementSelection();
Vehicle& rankSelection();


void crossOver(Vehicle&, Vehicle&, Vehicle&, Vehicle&);
void mutate(Vehicle&);
void updateFitness();




//---------------------------- WindowProc ---------------------------------//
//	This is the callback function which handles all the windows messages   //
//-------------------------------------------------------------------------//

LRESULT CALLBACK WindowProc(HWND   hwnd,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam)
{
	//create some pens to use for drawing
	static HPEN BluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	static HPEN RedPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	static HPEN OldPen = NULL;

	//create a solid brush
	static HBRUSH RedBrush = CreateSolidBrush(RGB(200, 128, 0));
	static HBRUSH YellowBrush = CreateSolidBrush(RGB(255, 255, 0));
	static HBRUSH ltBlueBrush = CreateSolidBrush(RGB(200, 200, 255));
	static HBRUSH ltGreenBrush = CreateSolidBrush(RGB(50, 255, 255));

	static HBRUSH OldBrush = NULL;

	//these hold the dimensions of the client window area
	static int cxClient, cyClient;


	//used to create the back buffer
	static HDC		hdcBackBuffer;
	static HBITMAP	hBitmap;
	static HBITMAP	hOldBitmap;

	switch (msg)
	{

		//A WM_CREATE msg is sent when your application window is first
		//created
	case WM_CREATE:
	{
		//to get get the size of the client window first we need  to create
		//a RECT and then ask Windows to fill in our RECT structure with
		//the client window size. Then we assign to cxClient and cyClient 
		//accordingly
		RECT rect;

		GetClientRect(hwnd, &rect);

		cxClient = rect.right;
		cyClient = rect.bottom;

		//seed random number generator
		srand((unsigned)time(NULL));

		//---------------create a surface for us to render to(backbuffer)

		//create a memory device context
		hdcBackBuffer = CreateCompatibleDC(NULL);

		//get the DC for the front buffer
		HDC hdc = GetDC(hwnd);

		hBitmap = CreateCompatibleBitmap(hdc,
			cxClient,
			cyClient);


		//select the bitmap into the memory device context
		hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

		//don't forget to release the DC
		ReleaseDC(hwnd, hdc);

		initGame();
	}

	break;

	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			PostQuitMessage(0);
		}

		break;
		}
	}

	case WM_PAINT:
	{

		PAINTSTRUCT ps;

		BeginPaint(hwnd, &ps);

		//fill our backbuffer with white
		BitBlt(hdcBackBuffer,
			0,
			0,
			cxClient,
			cyClient,
			NULL,
			NULL,
			NULL,
			WHITENESS);

		(HBRUSH)SelectObject(hdcBackBuffer, ltGreenBrush);

		Rectangle(hdcBackBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		drawGame(hdcBackBuffer);
		
		//now blit backbuffer to front
		BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY);

		EndPaint(hwnd, &ps);

		//delay a little
		Sleep(5);

	}

	break;

	//has the user resized the client area?
	case WM_SIZE:
	{
		//if so we need to update our variables so that any drawing
		//we do using cxClient and cyClient is scaled accordingly
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		//now to resize the backbuffer accordingly. First select
		//the old bitmap back into the DC
		SelectObject(hdcBackBuffer, hOldBitmap);

		//don't forget to do this or you will get resource leaks
		DeleteObject(hBitmap);

		//get the DC for the application
		HDC hdc = GetDC(hwnd);

		//create another bitmap of the same size and mode
		//as the application
		hBitmap = CreateCompatibleBitmap(hdc,
			cxClient,
			cyClient);

		ReleaseDC(hwnd, hdc);

		//select the new bitmap into the DC
		SelectObject(hdcBackBuffer, hBitmap);

	}

	break;

	case WM_DESTROY:
	{
		//delete the pens        
		DeleteObject(BluePen);
		DeleteObject(OldPen);

		//and the brushes
		DeleteObject(RedBrush);
		DeleteObject(OldBrush);

		//clean up our backbuffer objects
		SelectObject(hdcBackBuffer, hOldBitmap);

		DeleteDC(hdcBackBuffer);
		DeleteObject(hBitmap);

		// kill the application, this sends a WM_QUIT message  
		PostQuitMessage(0);
	}

	break;

	}//end switch

	 //this is where all the messages not specifically handled by our 
	 //winproc are sent to be processed
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//-------------------------------- WinMain -------------------------------
//
//	The entry point of the windows program
//------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     szCmdLine,
	int       iCmdShow)
{
	//handle to our window
	HWND						hWnd;

	//our window class structure
	WNDCLASSEX     winclass;

	// first fill in the window class stucture
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hInstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = NULL;
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = "GA_Car";
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//register the window class
	if (!RegisterClassEx(&winclass))
	{
		MessageBox(NULL, "Registration Failed!", "Error", 0);

		//exit the application
		return 0;
	}

	//create the window and assign its ID to hwnd    
	hWnd = CreateWindowEx(NULL,                 // extended style
		"GA_Car",  // window class name
		"GA Car",  // window caption
		WS_OVERLAPPEDWINDOW,  // window style
		0,                    // initial x position
		0,                    // initial y position
		WINDOW_WIDTH,         // initial x size
		WINDOW_HEIGHT,        // initial y size
		NULL,                 // parent window handle
		NULL,                 // window menu handle
		hInstance,            // program instance handle
		NULL);                // creation parameters

							  //make sure the window creation has gone OK
	if (!hWnd)
	{
		MessageBox(NULL, "CreateWindowEx Failed!", "Error!", 0);
	}

	//make the window visible
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	// Enter the message loop
	bool bDone = false;

	MSG msg;

	while (!bDone)
	{

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				// Stop loop if it's a quit message
				bDone = true;
			}

			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//this will call WM_PAINT which will render our scene
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);

		//*** your game loop goes here ***//
		updateGame();

	}//end while

	UnregisterClass("Backbuffer", winclass.hInstance);

	return msg.wParam;
}



void initGame()
{
	for (int i = 0; i < population; i++)
	{
		players[i].x = 200;
		players[i].y = 200;
	}

	for (int i = 0; i < numOfInst; i++)
	{
		instructions[i] = rand() % 4;
	}
}

void drawGame(HDC &hdcBackBuffer)
{
	//Loop through population and creating x amount of AI's on screen
	for (int i = 0; i < population; i++)
	{
		static HBRUSH ltBlueBrush = CreateSolidBrush(RGB(200, 200, 255));
		SelectObject(hdcBackBuffer, ltBlueBrush);
		Ellipse(hdcBackBuffer, players[i].x, players[i].y, players[i].x + 20, players[i].y + 20);
		MoveToEx(hdcBackBuffer, players[i].x + 10, players[i].y + 10, NULL); //+10 is offset
		LineTo(hdcBackBuffer, players[i].x + 10 + players[i].px, players[i].y + 10 + players[i].py);
	}

	//Drawing gameGoal (The object created above)
	Rectangle(hdcBackBuffer, gameGoal.x, gameGoal.y, gameGoal.x + gameGoal.width, gameGoal.y + gameGoal.height);

	//Draw text
	LPCSTR buffer = "AICar: ";
	TextOut(hdcBackBuffer, 5, 5, buffer, lstrlen(buffer));

	TCHAR buf[5];
	snprintf(buf, 5, "%f", playerRot);
	TextOut(hdcBackBuffer, 100, 5, buf, 5);
}


void updateGame()
{
	//getKeys(); //Manually control the game with arrow keys (Pointless)

	//When the game is complete refetch the AI population 
	if (gameComplete == false)
	{
		getAIPopulation();
	}

	//Loop through the population
	for (int i = 0; i < population; i++)
	{
		//If player x is greater than gameGoal x and player x is less than the gameGoal y + gameGoal width then.....
		if (players[i].x > gameGoal.x && players[i].x < gameGoal.x + gameGoal.width)
		{
			//If player y is greater than gameGoal y and player is less than gameGoal y + gameGoal height then....
			if (players[i].y > gameGoal.y && players[i].y < gameGoal.y + gameGoal.height)
			{
				gameComplete = true;
				MessageBox(NULL, "Success", "The AI has managed to hit the game goal", 0);
			}
		}
		return;

	}
}

void getAIPopulation()
{
	int currentInstructionruction = instructions[instPtr];
	if (currentInstruction < numOfInst)
	{
		currentInstruction++;
	}

	if (currentInstruction >= chromoLength)
	{
		evolve();
		generationCount++;

		for (int i = 0; i < population; i++)
		{
			players[i].x = 200;
			players[i].y = 200;
			players[i].currentInstruction = 0;

			players[i].px = 15;
			players[i].py = 0;
			players[i].playerRot = 0;
		}
		currentInstruction = 0;
	}


	for (int i = 0; i < population; i++) {

		int inst = players[i].instr[players[i].currentInstruction]; //Grabs current instructions
		
		//If the current instructions is less than the number of instructions then....
		if (players[i].currentInstruction < players[i].numOfInst) {
			players[i].currentInstruction++; //Gra
		} else {
			players[i].currentInstruction = 0; 
		}

		if (inst == 2) {
			players[i].playerRot += 0.1f;

			//2d point rotation - Radians not degrees
			players[i].px = 15 * cos(players[i].playerRot) - (0 * sin(players[i].playerRot));
			players[i].py = 15 * sin(players[i].playerRot) + (0 * cos(players[i].playerRot));
		}

		for (int i = 0; i < population; i++) {
			if (inst == 0) {
				players[i].playerRot -= 0.1f;
				players[i].px = 15 * cos(players[i].playerRot) - (0 * sin(players[i].playerRot));
				players[i].py = 15 * sin(players[i].playerRot) + (0 * cos(players[i].playerRot));
			}
		}

		for (int i = 0; i < population; i++) {
			if (inst == 1) {
				float x = players[i].x + (2 * cos(players[i].playerRot));
				float y = players[i].y + (2 * sin(players[i].playerRot));

				players[i].x = x;
				players[i].y = y;
			}
		}

		for (int i = 0; i < population; i++)
		{
			if (inst == 3)
			{
				float x = players[i].x + (-2 * cos(players[i].playerRot));
				float y = players[i].y + (-2 * sin(players[i].playerRot));

				players[i].x = x;
				players[i].y = y;
			}
		}
	}
}


//--------------
//EVOLVE 
//--------------

void evolve()
{
	updateFitness();
	sortPopulation();

	Vehicle babyPlayers[population];

	int newPop = 0;
	while (newPop<population)
	{
		//Vehicle mum = selection();
		//Vehicle dad = selection();
		Vehicle mum = tournementSelection();
		Vehicle dad = tournementSelection();
		//Vehicle mum = rankSelection();
		//Vehicle dad = rankSelection();

		Vehicle baby1, baby2;
		crossOver(mum, dad, baby1, baby2);

		//mutate
		mutate(baby1);
		mutate(baby2);

		//copy babys into babypopulation
		babyPlayers[newPop] = baby1;
		babyPlayers[newPop + 1] = baby2;
		newPop += 2;
	}

	for (int i = 0; i<population; i++)
	{
		players[i] = babyPlayers[i];
	}

}
void decode(Vehicle)
{
	//convert instructions to someGoal usable  
	//not needed? 
}


//-------------------
// Update Fitness
//-------------------
void updateFitness()
{
	totalFitness = 0;

	for (int i = 0; i<population; i++)
	{
		//plant fitness between 0 and 1 based on height. Max 290 units above pot.
		//players[i].fitness = ((100 / maxHeight)*players[i].height) / 100;

		/*
		float diffx = abs(players[i].x - gameGoal.x);
		float diffy = abs(players[i].y - gameGoal.y);
		double fit = 1 / (diffx + diffy + 1);	
		*/
	
		double fit = 1 / (double)(480 - players[i].x + 1);

		
		players[i].fitness = fit;

		totalFitness += fit;
	}
}


//-------------
//Selection
//------------
Vehicle& selection()
{
	float fSlice = (rand()) / (RAND_MAX + 1.0)* totalFitness;
	//float fSlice = (rand()* totalFitness);
	float total = 0;
	int selectedPlayer = 0;

	for (int i = 0; i<population; i++)
	{
		total += players[i].fitness;

		if (total>fSlice)
		{
			selectedPlayer = i;
			break;
		}
	}
	return players[selectedPlayer];
}

//Tournement Selection
Vehicle& tournementSelection()
{
	int selectedPlayer = 0;

	int p1 = (rand() % population);
	int p2 = (rand() % population);

	if (players[p1].fitness > players[p2].fitness)
	{
		return players[p1];
	}
	else {
		return players[p2];
	}

}

void sortPopulation()
{
	//sort in desceding order... To be Checked!! 
	int i, j;
	for (i = 0; i < population - 1; i++)
	{
		for (j = 0; j < population - i - 1; j++)
		{
			if (players[j].fitness < players[j + 1].fitness)
			{
				//swap
				Vehicle temp = players[j];
				players[j] = players[j + 1];
				players[j + 1] = temp;
			}
		}
	}
	//set rank fitness based on position of player
	for (int i = 0; i < population; i++)
	{
		double temp = (i + 1) / (double)population; //add 1 to stop divide by zero
		players[i].rankFitness = temp;
	}

}


//Rank selection
Vehicle& rankSelection()
{
	float fSlice = (rand()) / (RAND_MAX + 1.0)* population;
	float total = 0;
	int selectedPlayer = 0;

	for (int i = 0; i < population; i++)
	{
		total += players[i].rankFitness; //Total is set to the players rank fitness
		if (total > fSlice)
		{
			selectedPlayer = i;
			break;
		}
	}
	return players[selectedPlayer];
}


//-----------------
//Crossover
//-------------------
void crossOver(Vehicle&mum, Vehicle&dad, Vehicle&baby1, Vehicle&baby2)
{
	if (((rand()) / (RAND_MAX + 1.0)>CROSSOVER_RATE) || mum == dad)
	{
		baby1 = mum;
		baby2 = dad;

		return;
	}

	int xover = rand() % chromoLength;

	for (int i = 0; i<xover; i++)
	{
		baby1.instr[i] = mum.instr[i];
		baby2.instr[i] = dad.instr[i];
	}

	for (int i = xover; i<chromoLength; i++)
	{
		baby1.instr[i] = dad.instr[i];
		baby2.instr[i] = mum.instr[i];
	}
}

//------------
//MUTATE
//------------
void mutate(Vehicle& currPlayer)
{
	for (int bit = 0; bit<chromoLength; bit++)
	{
		if (((rand()) / (RAND_MAX + 1.0)<MUTATION_RATE))
		{
			//original
			//currPlayer.instr[bit] = (rand() % 4);//!currPlayer.instr[bit];

			//new - prevents same instruction from being chosen. 
			int newInst = 0;
			do
			{
				newInst = (rand() % 4);
				//if (newInst == 1) { newInst = 2; }
			} while (newInst == currPlayer.instr[bit]);
			currPlayer.instr[bit] = newInst;

		}
	}
}


//Keyboard input
void getKeys()
{
	
	if (KEYDOWN(VK_RIGHT) && playerx<600)
	{
		//playerx++;
		playerRot+=0.1f;
		//2d point rotation - Radians not degrees
		px = 15 * cos(playerRot) - (0 * sin(playerRot));
		py = 15 * sin(playerRot) + (0 * cos(playerRot));

	}
	if (KEYDOWN(VK_LEFT) && playerx>0)
	{
		//playerx--;
		playerRot -= 0.1f;
		px = 15 * cos(playerRot) - (0 * sin(playerRot));
		py = 15 * sin(playerRot) + (0 * cos(playerRot));

	}

	if (KEYDOWN(VK_UP) && playery>0)
	{
		//translate 2d point
		//newx = oldx + (velocity*cos(angle))
		//newy = oldy + (velocity*sin(angle))
		float x = playerx + (2 * cos(playerRot));
		float y = playery + (2 * sin(playerRot));

		playerx = x;
		playery = y;
	}
	if (KEYDOWN(VK_DOWN) && playery<600)
	{
		playery++;
	}

	if (KEYDOWN(VK_SPACE))
	{
	}
	if (KEYDOWN(VK_RETURN))
	{
		initGame();
	}
	// https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
	if (KEYDOWN(0x52)) //'R'
	{
		initGame();
	}

	if (KEYDOWN(0x53)) //'S'
	{
	}
}