//A* Pathfinding Algorithm

/*
	GridSquare class:
		x and y variables to indicate position on grid at Grid[x][y]
		count variable to count number of moves from starting position to get to this square
		blocked bool to indicate if square is blocked or has been used already
*/

/*
	AStarPathfinder class:
	
		openList is STL priority queue used to store GridSquares that moves will be made from.
			Moves are added to the queue using addtoopenList in order of their pathscore and taken using getfromopenList.
			isopenListEmpty returns true if openList size is 0
			
		SetStartandEnd copies the start and end GridSquares as variables and adds the start square to the openList
	
		isThereAPath takes square from openList and checks adjacent squares in 8 directions
			isLastSquare used to check adjacent squares if checks say available - if not, add new square to openList
			if last square is found, return true - path found
			If openList is empty and no path found, return false
		
		DisplayGrid prints out the count number for each square in the grid

		check functions check adjacent square in each direction to see if on grid and unblocked - return true, else false

		isLastSquare checks square against gs_last variable to determine if path has been found
*/
		

#include <iostream>
#include <ctime>
#include <queue>
#include <windows.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <string>
#include <stdio.h>

using namespace std;

#define GRID_X 10
#define GRID_Y 10

CONSOLE_SCREEN_BUFFER_INFO con_info;   // holds screen info
HANDLE hconsole;         // handle to console

//timer variables
unsigned long t;
double time_taken;

//GridSquare class to hold the variables associated with each GridSquare 
class GridSquare
{
public:
	GridSquare();
	int x, y; //position variables
	int count; //move count
	int moveScore, pathScore;
	bool on_path; //for displaying the actual path
};

//A* PathFinder algorithm functions
class AStarPathfinder
{
public:
	void SetStartandEnd(GridSquare f, GridSquare l); //takes user input squares and sets them as the start and end points
	int isThereAPath(); //pulls move from openList, checks adjacent squares, adds possible moves to openList. 
						//Returns number of moves when end point is reached - 0 means no path found - no more legal moves
	void TraceBack(); //checks adjacent squares for reducing move score until first square is found
					  //the path found could vary depending on the order in which these checks are done
					  //currently favours west, then south, then east, then north

	//compares coordinates of g against first/last square
	bool isFirstSquare(GridSquare g); 
	bool isLastSquare(GridSquare g);
	
private:
	void getHeuristic(GridSquare & g);

	void DisplayGrid(); //displays move scores for each score as they are updated and highlights squares on the path found
	
	//checks on adjacent squares in 8 directions to see if move is legal
	bool checkSquares(GridSquare g); 

	//checks adjacent squares in the 8 directions to see if move score has reduced by one
	bool traceSquares(GridSquare & g);

	//push_back() copy of square to openList deque - also copies square to grid
	void addtoopenList(GridSquare g);
	//returns copy of first element of openList and then pops it
	GridSquare getfromopenList();
	//returns true if openList.size() is 0
	bool isopenListEmpty();
	//copies square to grid
	void addtoGrid(GridSquare g);

	//variables
	GridSquare gs_first;
	GridSquare gs_last;
};

//struct used to return the class object with the highest pathscore - we want to sort the openlist so that the lowest (best score) is last
//in order to push it to select it next for finding path
class Compare_pathScores 
{
public:
	inline bool operator() (const GridSquare & a, const GridSquare & b)
	{
		return a.pathScore > b.pathScore;
	}
};

//graphics/set up function prototypes
void Init_Graphics(void);
inline void Set_Color(int fcolor, int bcolor);
inline void Draw_String(int x,int y, char *string);
inline void Clear_Screen(void);

priority_queue<GridSquare, deque<GridSquare>, Compare_pathScores> openList;

GridSquare Grid[GRID_X][GRID_Y]; //grid used to store all GridSquares

int main()
{
	//variables for user entered first and last squares for path
	GridSquare first;
	GridSquare last;
	int number_of_moves; //number of moves taken to reach goal

	AStarPathfinder pathfinder; //object for all pathfinding functions

	for(int i=0; i<GRID_X; i++) //set coordinates for grid squares
	{
		for(int j=0; j<GRID_Y; j++)
		{
			Grid[i][j].x = i;
			Grid[i][j].y = j;
		}
	}
	
	// set up the console text graphics system
    Init_Graphics();

    // clear the screen
    Clear_Screen();

	Draw_String(0, 0, "\t\tA-Star Pathfinding Algorithm");
	//get user input for start and end squares
	Draw_String(0, 2, "Enter x-coordinate for starting point (0-9): ");
	cin >> first.x;

	Draw_String(0, 3, "Enter y-coordinate for starting point (0-9): ");
	cin >> first.y;

	Draw_String(0, 4, "Enter x-coordinate for finishing point (0-9): ");
	cin >> last.x;

	Draw_String(0, 5, "Enter y-coordinate for finishing point (0-9): ");
	cin >> last.y;

	t = clock();
	
	pathfinder.SetStartandEnd(first, last); //store copy of start and end squares - will update grid

	//adds some blocked squares to test checks in place
	for(int blk = 3; blk < 8; blk++)
	{
		if(!pathfinder.isFirstSquare(Grid[4][blk]))
		{
			if(!pathfinder.isLastSquare(Grid[4][blk]))
				Grid[4][blk].count = -1;
		}
	}

	number_of_moves = pathfinder.isThereAPath();
	Set_Color(15,0);
	if(number_of_moves>0)
	{
		Draw_String(45, 14, "Path found in ");
		cout << (number_of_moves-1) << " moves.";
	}
	else
	{
		Draw_String(45, 14, "No path found.");
	}

	pathfinder.TraceBack();

	time_taken = ((double)(clock() - t)) / CLOCKS_PER_SEC;
	//Draw_String(45, 17, "Time spent = ");
	//cout << time_taken << endl;

	Sleep(100000);

	return 0;
}


GridSquare::GridSquare()
{
	count = 0;
	on_path = false;
}


void AStarPathfinder::SetStartandEnd(GridSquare f, GridSquare l)
{
	gs_first = f;
	gs_first.on_path = true;
	gs_first.count++;
	addtoopenList(gs_first);

	gs_last = l;
	gs_last.on_path = true;
	addtoGrid(gs_last);

}

int AStarPathfinder::isThereAPath()
{
	GridSquare theSquare;
	GridSquare nextSquare;
	int xmin, xmax; //variables to find the adjacent squares on the grid
	int ymin, ymax;
	
	while(!isopenListEmpty())
	{
		theSquare = getfromopenList();

		theSquare.on_path = false;
	
		//checks to ensure it does not look at squares off the edge of the grid
		if(theSquare.x == 0)
			xmin = 0;
		else
		{
			xmin = theSquare.x - 1;
		}
		if(theSquare.x == GRID_X-1)
			xmax = GRID_X;
		else
		{
			xmax = theSquare.x + 2;
		}
		if(theSquare.y == 0)
			ymin = 0;
		else
		{
			ymin = theSquare.y - 1;
		}
		if(theSquare.y == GRID_Y-1)
			ymax = GRID_Y;
		else
		{
			ymax = theSquare.y + 2;
		}
		
		for(int x = xmin; x<xmax; x++)
		{
			for(int y = ymin; y<ymax; y++)
			{
				if(checkSquares(Grid[x][y]))
				{
					nextSquare.x = x;
					nextSquare.y = y;
					nextSquare.count = theSquare.count+1;

					if(isLastSquare(nextSquare))
					{
						gs_last = nextSquare;
						gs_last.on_path = true;
						addtoGrid(gs_last);
						return nextSquare.count;
					}
					else
					{
						addtoopenList(nextSquare);
						DisplayGrid();
					}
				}
			}
		}
	}
	return 0;
}

void AStarPathfinder::TraceBack()
{
	GridSquare aSquare;
	aSquare = gs_last;

	while(!isFirstSquare(aSquare))
	{
		if(traceSquares(aSquare))
		{
			addtoGrid(aSquare);
			DisplayGrid();
		}
	}
}

bool AStarPathfinder::isFirstSquare(GridSquare g)
{
	if(g.x == gs_first.x && g.y == gs_first.y)
		return true;
	else
	{
		return false;
	}
}

bool AStarPathfinder::isLastSquare(GridSquare g)
{
	if(g.x == gs_last.x && g.y == gs_last.y)
		return true;
	else
	{
		return false;
	}
}

//uses a diagonal shortcut method to calculate the pathscore for given grid square
void AStarPathfinder::getHeuristic(GridSquare & g)
{	
	//variables
	int destScore, moveScore;

	int x_dest, y_dest;
	int diag_dest, hori_dest;

	x_dest = abs(gs_last.x-g.x);
	y_dest = abs(gs_last.y-g.y);
	
	diag_dest = x_dest < y_dest ? x_dest*14 : y_dest*14;
	hori_dest = abs(x_dest-y_dest)*10;
	
	destScore = diag_dest + hori_dest;

	if(isFirstSquare(g))
	{
		g.moveScore = 0;
	}
	else
	{
		GridSquare temp = g;
		traceSquares(temp);

		if(g.x==temp.x || g.y==temp.y)
			g.moveScore = 10 + temp.moveScore;
		else
		{
			g.moveScore = 14 + temp.moveScore;
		}
	}

	g.pathScore = destScore + g.moveScore;
}

void AStarPathfinder::DisplayGrid()
{
	for(int i=0; i<GRID_X; i++)
	{
		for(int j=0; j<GRID_Y; j++)
		{
			if(Grid[i][j].on_path)
			{
				Set_Color(13,0);
				Draw_String((i*3)+3, (j*2)+6, " * "); 
			}
			else if(Grid[i][j].count>0)
			{
				Set_Color(14,0);
				Draw_String((i*3)+3, (j*2)+6, "");
				cout << " " << (Grid[i][j].count-1);
			}
		}
	}
	Sleep(300);
}

bool AStarPathfinder::checkSquares(GridSquare g)
{
	if(g.count == 0)  //checks if the square is blocked/already used on the grid
	{
		return true;
	}
	return false; 
}

bool AStarPathfinder::traceSquares(GridSquare & g)
{
	int xmin, xmax; //variables to find the adjacent squares on the grid
	int ymin, ymax;
	
	//checks to ensure it does not look at squares off the edge of the grid
	if(g.x == 0)
		xmin = 0;
	else
	{
		xmin = g.x - 1;
	}
	if(g.x == GRID_X-1)
		xmax = GRID_X;
	else
	{
		xmax = g.x + 2;
	}
	if(g.y == 0)
		ymin = 0;
	else
	{
		ymin = g.y - 1;
	}
	if(g.y == GRID_Y-1)
		ymax = GRID_Y;
	else
	{
		ymax = g.y + 2;
	}
	
	for(int x = xmin; x<xmax; x++)
	{
		for(int y = ymin; y<ymax; y++)
		{
			if(Grid[x][y].count == g.count-1)  //checks if the square is blocked/already used on the grid
			{
				g.x = x;
				g.y = y;
				g.count--;
				g.on_path = true;
				return true;
			}
		}
	}
	return false; 
}

void AStarPathfinder::addtoopenList(GridSquare g)
{
	getHeuristic(g);
	addtoGrid(g);
	openList.push(g);
}

GridSquare AStarPathfinder::getfromopenList()
{
	GridSquare nextSquare;

	nextSquare = openList.top();
	openList.pop();

	return nextSquare;
}

bool AStarPathfinder::isopenListEmpty()
{
	if(openList.size() == 0)
		return true;
	else
	{
		return false;
	}
}

void AStarPathfinder::addtoGrid(GridSquare g)
{
	Grid[g.x][g.y] = g;
}

/*Graphics functions taken from CJM's
  Game program adaptation of
  Andre La Mothe's book
  The Black Art of 3D Games Programming*/

// this function initializes the console graphics engine
void Init_Graphics(void){
     COORD console_size = {80,25}; // size of console
    // open i/o channel to console screen
    hconsole = CreateFile(TEXT("CONOUT$"), GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 0L, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);
    // set screensize
    SetConsoleScreenBufferSize(hconsole,console_size);
    // get details for console screen
    GetConsoleScreenBufferInfo(hconsole, &con_info);
} // end Init_Graphics

// this function sets the color of the console output
inline void Set_Color(int fcolor, int bcolor=0){
    SetConsoleTextAttribute(hconsole,(WORD)((bcolor << 4) | fcolor));
} // Set_Color

// this function draws a string at the given x,y
inline void Draw_String(int x, int y, char *string){
    COORD cursor_pos; // used to pass coords
    // set printing position
    cursor_pos.X = x;
    cursor_pos.Y = y;
    SetConsoleCursorPosition(hconsole, cursor_pos);
    // print the string in current color
    cout << string;
} // end Draw_String

// this function clears the screen
inline void Clear_Screen(void){
    // set colour to white on black
    Set_Color(15,0);
    // clear the screen
	for (int x=0; x<=80; x++){
		for (int y=0; y<=25; y++){
        Draw_String(x, y, " ");
		}
	}
} // end Clear_Screen
