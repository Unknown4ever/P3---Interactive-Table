#include <iostream>
#include <thread>
#include <chrono>
#include <string> 
#include <dos.h> //for delay
#include <conio.h> //for getch()
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

const int BOARD_WIDTH = 20;
const int BOARD_HEIGHT = 10;
const int SHIP_TYPES = 4;

const char WATER = '~';
const char HIT = 'X';
const char is_SHIP = 'S';
const char MISS = 'O';

bool scoutArea = false;
bool multiShot = false;
bool superMultiShot = false;
bool strafeRun = false;
bool regularShot = false;

//Function for loading and displaying images
void displayImage(string image_name) {
	Mat img;
	img = imread(image_name);
	imshow(image_name, img);
	waitKey(0);
}

struct POINT {
	//Horizontal and vertical coordinates
	int X = 0;
	int Y = 0;
};

struct SHIP {
	//Ship name
	string name;
	//Total points on the grid
	int length;
	//Coordinates of those points
	POINT onGrid[4]; //0-5 max length of biggest ship
	//Whether or not those points are a "hit"
	bool hitFlag[4];
}ship[SHIP_TYPES];

struct PLAYER {
	char grid[BOARD_WIDTH][BOARD_HEIGHT];
}player[3];

enum DIRECTION { //The enum direction can either get the value horizontal or vertical
	HORIZONTAL,
	VERTICAL
};

struct PLACESHIPS {
	DIRECTION direction;
	SHIP shipType;
};

bool guessingStage = false;

//Functions
void LoadShips();
void ResetBoard();
void DrawBoard(int);
PLACESHIPS UserInputShipPlacement();
bool UserInputAttack(int&, int&, int);
bool GameOverCheck(int);

int main()
{
	LoadShips();
	ResetBoard();

	//insert function here in the future for chaning between abilities
	regularShot = false;
	multiShot = true;
	superMultiShot = false;
	strafeRun = false;
	scoutArea = false;

	//displayImage("grid.png");
	//displayImage("carrier.png");
	//Loop through each player and place ships
	for (int aPlayer = 1; aPlayer < 3; aPlayer++)
	{
		//Loop through each ship type to place
		for (int this_SHIP = 0; this_SHIP < SHIP_TYPES; ++this_SHIP)
		{
			//Display board
			system("cls");
			DrawBoard(aPlayer); //Displaying the board for ship placement stage

			//Instructions
			cout << "\n";
			cout << "(Player " << aPlayer << ")\n\n";
			cout << "Horizontal: 0, Vetical: 1, then enter X and Y coordinates\n";
			cout << "Ship to place: " << ship[this_SHIP].name << " which has a length of " << ship[this_SHIP].length << "\n";
			cout << "Where do you want it placed? (specify direction first): \n";
			

			//Get input from user and loop until good data is returned
			PLACESHIPS aShip;
			aShip.shipType.onGrid[0].X = -1;
			while (aShip.shipType.onGrid[0].X == -1)
			{
				aShip = UserInputShipPlacement();
			}

			//Combine user data with "this ship" data
			aShip.shipType.length = ship[this_SHIP].length;
			aShip.shipType.name = ship[this_SHIP].name;

			//Add the FIRST grid point to the current player's game board
			player[aPlayer].grid[aShip.shipType.onGrid[0].X][aShip.shipType.onGrid[0].Y] = is_SHIP;

			//Determine ALL grid points based on length and direction
			for (int i = 1; i < aShip.shipType.length; ++i)
			{
				if (aShip.direction == HORIZONTAL) {
					aShip.shipType.onGrid[i].X = aShip.shipType.onGrid[i - 1].X + 1;
					aShip.shipType.onGrid[i].Y = aShip.shipType.onGrid[i - 1].Y;
				}
				if (aShip.direction == VERTICAL) {
					aShip.shipType.onGrid[i].Y = aShip.shipType.onGrid[i - 1].Y + 1;
					aShip.shipType.onGrid[i].X = aShip.shipType.onGrid[i - 1].X;
				}

				//Add the REMAINING grid points to our current players game board
				player[aPlayer].grid[aShip.shipType.onGrid[i].X][aShip.shipType.onGrid[i].Y] = is_SHIP;

			}
			//Loop back through each ship type
		}
		//Loop back through each player
	}

	//Game part after ships have been placed by both players
	guessingStage = true;
	int thisPlayer = 1;
	do {
		//Because we are ATTACKING now, the opposite players board is the display board
		int enemyPlayer;
		if (thisPlayer == 1) enemyPlayer = 2;
		if (thisPlayer == 2) enemyPlayer = 1;
		system("cls");
		DrawBoard(enemyPlayer);


		//Get attack coords from this player
		bool goodInput = false;
		int x, y;
		while (goodInput == false) {
			goodInput = UserInputAttack(x, y, thisPlayer);
		}
		
													// ABILITIES //

		//Regular shot
		
		if (regularShot == true) {

			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = HIT;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "BOOM"; std::this_thread::sleep_for(std::chrono::seconds(3));
			}
			if (player[enemyPlayer].grid[x][y] == WATER) {
				player[enemyPlayer].grid[x][y] = MISS;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "MISS";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}

		}

		//Scout the enemy board and displays the ships for a few seconds
		//Will be true when the correct shape is detected by the camera

		if (scoutArea == true) {

			if (player[enemyPlayer].grid[x - 1][y] == is_SHIP) { player[enemyPlayer].grid[x - 1][y] = 's'; }
			if (player[enemyPlayer].grid[x + 1][y] == is_SHIP) { player[enemyPlayer].grid[x + 1][y] = 's'; }
			if (player[enemyPlayer].grid[x][y - 1] == is_SHIP) { player[enemyPlayer].grid[x][y - 1] = 's'; }
			if (player[enemyPlayer].grid[x][y + 1] == is_SHIP) { player[enemyPlayer].grid[x][y + 1] = 's'; }

			if (player[enemyPlayer].grid[x + 2][y - 1] == is_SHIP) { player[enemyPlayer].grid[x + 2][y - 1] = 's'; }
			if (player[enemyPlayer].grid[x + 3][y - 1] == is_SHIP) { player[enemyPlayer].grid[x + 3][y - 1] = 's'; }

			if (player[enemyPlayer].grid[x + 2][y] == is_SHIP) { player[enemyPlayer].grid[x + 2][y] = 's'; }
			if (player[enemyPlayer].grid[x + 3][y] == is_SHIP) { player[enemyPlayer].grid[x + 3][y] = 's'; }

			if (player[enemyPlayer].grid[x + 2][y + 1] == is_SHIP) { player[enemyPlayer].grid[x + 2][y + 1] = 's'; }
			if (player[enemyPlayer].grid[x + 3][y + 1] == is_SHIP) { player[enemyPlayer].grid[x + 3][y + 1] = 's'; }

			if (player[enemyPlayer].grid[x - 1][y] == WATER) { player[enemyPlayer].grid[x - 1][y] = '~'; }
			if (player[enemyPlayer].grid[x + 1][y] == WATER) { player[enemyPlayer].grid[x + 1][y] = '~'; }
			if (player[enemyPlayer].grid[x][y - 1] == WATER) { player[enemyPlayer].grid[x][y - 1] = '~'; }
			if (player[enemyPlayer].grid[x][y + 1] == WATER) { player[enemyPlayer].grid[x][y + 1] = '~'; }

			if (player[enemyPlayer].grid[x + 2][y - 1] == WATER) { player[enemyPlayer].grid[x + 2][y - 1] = '~'; }
			if (player[enemyPlayer].grid[x + 3][y - 1] == WATER) { player[enemyPlayer].grid[x + 3][y - 1] = '~'; }

			if (player[enemyPlayer].grid[x + 2][y] == WATER) { player[enemyPlayer].grid[x + 2][y] = '~'; }
			if (player[enemyPlayer].grid[x + 3][y] == WATER) { player[enemyPlayer].grid[x + 3][y] = '~'; }

			if (player[enemyPlayer].grid[x + 2][y + 1] == WATER) { player[enemyPlayer].grid[x + 2][y + 1] = '~'; }
			if (player[enemyPlayer].grid[x + 3][y + 1] == WATER) { player[enemyPlayer].grid[x + 3][y + 1] = '~'; }


			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = 's';
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "Scanning for enemy ships";
				std::this_thread::sleep_for(std::chrono::seconds(4));
			}

			if (player[enemyPlayer].grid[x][y] == WATER) {
				player[enemyPlayer].grid[x][y] = '~';
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "Scanning for enemy ships";
				std::this_thread::sleep_for(std::chrono::seconds(4));
			}

			//Reseting the board after the scout ability
			if (player[enemyPlayer].grid[x - 1][y] == 's') { player[enemyPlayer].grid[x - 1][y] = is_SHIP; }
			if (player[enemyPlayer].grid[x + 1][y] == 's') { player[enemyPlayer].grid[x + 1][y] = is_SHIP; }
			if (player[enemyPlayer].grid[x][y - 1] == 's') { player[enemyPlayer].grid[x][y - 1] = is_SHIP; }
			if (player[enemyPlayer].grid[x][y + 1] == 's') { player[enemyPlayer].grid[x][y + 1] = is_SHIP; }

			if (player[enemyPlayer].grid[x + 2][y - 1] == 's') { player[enemyPlayer].grid[x + 2][y - 1] = is_SHIP; }
			if (player[enemyPlayer].grid[x + 3][y - 1] == 's') { player[enemyPlayer].grid[x + 3][y - 1] = is_SHIP; }

			if (player[enemyPlayer].grid[x + 2][y] == 's') { player[enemyPlayer].grid[x + 2][y] = is_SHIP; }
			if (player[enemyPlayer].grid[x + 3][y] == 's') { player[enemyPlayer].grid[x + 3][y] = is_SHIP; }

			if (player[enemyPlayer].grid[x + 2][y + 1] == 's') { player[enemyPlayer].grid[x + 2][y + 1] = is_SHIP; }
			if (player[enemyPlayer].grid[x + 3][y + 1] == 's') { player[enemyPlayer].grid[x + 3][y + 1] = is_SHIP; }

			if (player[enemyPlayer].grid[x - 1][y] == '~') { player[enemyPlayer].grid[x - 1][y] = WATER; }
			if (player[enemyPlayer].grid[x + 1][y] == '~') { player[enemyPlayer].grid[x + 1][y] = WATER; }
			if (player[enemyPlayer].grid[x][y - 1] == '~') { player[enemyPlayer].grid[x][y - 1] = WATER; }
			if (player[enemyPlayer].grid[x][y + 1] == '~') { player[enemyPlayer].grid[x][y + 1] = WATER; }

			if (player[enemyPlayer].grid[x + 2][y - 1] == '~') { player[enemyPlayer].grid[x + 2][y - 1] = WATER; }
			if (player[enemyPlayer].grid[x + 3][y - 1] == '~') { player[enemyPlayer].grid[x + 3][y - 1] = WATER; }

			if (player[enemyPlayer].grid[x + 2][y] == '~') { player[enemyPlayer].grid[x + 2][y] = WATER; }
			if (player[enemyPlayer].grid[x + 3][y] == '~') { player[enemyPlayer].grid[x + 3][y] = WATER; }

			if (player[enemyPlayer].grid[x + 2][y + 1] == '~') { player[enemyPlayer].grid[x + 2][y + 1] = WATER; }
			if (player[enemyPlayer].grid[x + 3][y + 1] == '~') { player[enemyPlayer].grid[x + 3][y + 1] = WATER; }

			if (player[enemyPlayer].grid[x][y] == 's') {
				player[enemyPlayer].grid[x][y] = is_SHIP;
			}

			if (player[enemyPlayer].grid[x][y] == '~') {
				player[enemyPlayer].grid[x][y] = WATER;
			}
	}

		if (multiShot == true) {

			if (player[enemyPlayer].grid[x - 1][y] == is_SHIP) { player[enemyPlayer].grid[x - 1][y] = HIT; }
			if (player[enemyPlayer].grid[x + 1][y] == is_SHIP) { player[enemyPlayer].grid[x + 1][y] = HIT; }
			if (player[enemyPlayer].grid[x][y - 1] == is_SHIP) { player[enemyPlayer].grid[x][y - 1] = HIT; }
			if (player[enemyPlayer].grid[x][y + 1] == is_SHIP) { player[enemyPlayer].grid[x][y + 1] = HIT; }

			if (player[enemyPlayer].grid[x - 1][y] == WATER) { player[enemyPlayer].grid[x - 1][y] = MISS; }
			if (player[enemyPlayer].grid[x + 1][y] == WATER) { player[enemyPlayer].grid[x + 1][y] = MISS; }
			if (player[enemyPlayer].grid[x][y - 1] == WATER) { player[enemyPlayer].grid[x][y - 1] = MISS; }
			if (player[enemyPlayer].grid[x][y + 1] == WATER) { player[enemyPlayer].grid[x][y + 1] = MISS; }

			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = HIT;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "BOOM";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}	

			if (player[enemyPlayer].grid[x][y] == WATER) {
				player[enemyPlayer].grid[x][y] = MISS;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "MISS";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
		}

		 //Shoots in a 3x3 square

		if (superMultiShot == true) {

			if (player[enemyPlayer].grid[x - 1][y] == is_SHIP) { player[enemyPlayer].grid[x - 1][y] = HIT; }
			if (player[enemyPlayer].grid[x + 1][y] == is_SHIP) { player[enemyPlayer].grid[x + 1][y] = HIT; }
			if (player[enemyPlayer].grid[x][y - 1] == is_SHIP) { player[enemyPlayer].grid[x][y - 1] = HIT; }
			if (player[enemyPlayer].grid[x][y + 1] == is_SHIP) { player[enemyPlayer].grid[x][y + 1] = HIT; }
			if (player[enemyPlayer].grid[x - 1][y - 1] == is_SHIP) { player[enemyPlayer].grid[x - 1][y - 1] = HIT; }
			if (player[enemyPlayer].grid[x + 1][y - 1] == is_SHIP) { player[enemyPlayer].grid[x + 1][y - 1] = HIT; }
			if (player[enemyPlayer].grid[x - 1][y + 1] == is_SHIP) { player[enemyPlayer].grid[x - 1][y + 1] = HIT; }
			if (player[enemyPlayer].grid[x + 1][y + 1] == is_SHIP) { player[enemyPlayer].grid[x + 1][y + 1] = HIT; }

			if (player[enemyPlayer].grid[x - 1][y] == WATER) { player[enemyPlayer].grid[x - 1][y] = MISS; }
			if (player[enemyPlayer].grid[x + 1][y] == WATER) { player[enemyPlayer].grid[x + 1][y] = MISS; }
			if (player[enemyPlayer].grid[x][y - 1] == WATER) { player[enemyPlayer].grid[x][y - 1] = MISS; }
			if (player[enemyPlayer].grid[x][y + 1] == WATER) { player[enemyPlayer].grid[x][y + 1] = MISS; }
			if (player[enemyPlayer].grid[x - 1][y - 1] == WATER) { player[enemyPlayer].grid[x - 1][y - 1] = MISS; }
			if (player[enemyPlayer].grid[x + 1][y - 1] == WATER) { player[enemyPlayer].grid[x + 1][y - 1] = MISS; }
			if (player[enemyPlayer].grid[x - 1][y + 1] == WATER) { player[enemyPlayer].grid[x - 1][y + 1] = MISS; }
			if (player[enemyPlayer].grid[x + 1][y + 1] == WATER) { player[enemyPlayer].grid[x + 1][y + 1] = MISS; }

			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = HIT;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "BOOM";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}

			if (player[enemyPlayer].grid[x][y] == WATER) {
				player[enemyPlayer].grid[x][y] = MISS;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "MISS";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
		}

		 //Shoots in a line

		if (strafeRun == true) {

			if (player[enemyPlayer].grid[x - 1][y] == is_SHIP) { player[enemyPlayer].grid[x - 1][y] = HIT; }
			if (player[enemyPlayer].grid[x + 1][y] == is_SHIP) { player[enemyPlayer].grid[x + 1][y] = HIT; }
			if (player[enemyPlayer].grid[x - 2][y] == is_SHIP) { player[enemyPlayer].grid[x - 2][y] = HIT; }
			if (player[enemyPlayer].grid[x + 2][y] == is_SHIP) { player[enemyPlayer].grid[x + 2][y] = HIT; }
			
			if (player[enemyPlayer].grid[x - 1][y] == WATER) { player[enemyPlayer].grid[x - 1][y] = MISS; }
			if (player[enemyPlayer].grid[x + 1][y] == WATER) { player[enemyPlayer].grid[x + 1][y] = MISS; }
			if (player[enemyPlayer].grid[x - 2][y] == WATER) { player[enemyPlayer].grid[x - 2][y] = MISS; }
			if (player[enemyPlayer].grid[x + 2][y] == WATER) { player[enemyPlayer].grid[x + 2][y] = MISS; }
			
			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = HIT;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "BOOM";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}

			if (player[enemyPlayer].grid[x][y] == WATER) {
				player[enemyPlayer].grid[x][y] = MISS;
				system("cls");
				DrawBoard(enemyPlayer);
				cout << "MISS";
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
		}

												//END OF ABILITIES //

		//If 0 is returned -> nobody has won yet
		int aWin = GameOverCheck(enemyPlayer);
		if (aWin != 0) {
			guessingStage = false;
			break;
		}
		//std::this_thread::sleep_for(std::chrono::seconds(3));

		//Switching between the two players
		thisPlayer = (thisPlayer == 1) ? 2 : 1;
	} while (guessingStage);

	system("cls");
	cout << "\n\nPLAYER " << thisPlayer << "WON\n\n\n\n";

	system("pause");
	return 0;
}


bool GameOverCheck(int enemyPLAYER)
{
	bool winner = true;
	//Loop through enemy board
	for (int w = 0; w < BOARD_WIDTH; w++) {
		for (int h = 0; h < BOARD_HEIGHT; h++) {
			//If any ships remain, game is still not over
			if (player[enemyPLAYER].grid[w][h] = is_SHIP)
			{
				winner = false;
				return winner;
			}
		}
	}
	//To know when is the end of the game
	return winner;
}


bool UserInputAttack(int& x, int& y, int theplayer)
{
	if (scoutArea == true) {
		cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO SCOUT: ";
	}
	else if (multiShot == true) {
		cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO MULTISHOT: ";
	}
	else if (superMultiShot == true) {
		cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO SUPERMULTISHOT: ";
	}
	else if (strafeRun == true) {
		cout << "\nPLAYER" << theplayer << ", ENTER COORDINATES TO STRAFE RUN: ";
	}
	else {
		cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO ATTACK: ";
	}

	bool goodInput = false;
	cin >> x >> y;
	if (x < 0
		|| x >= BOARD_WIDTH) return goodInput;
	if (y < 0 || y >= BOARD_HEIGHT) return goodInput;
	goodInput = true;
	//delay(500);

	return goodInput;
}

PLACESHIPS UserInputShipPlacement()
{
	int d, x, y;
	PLACESHIPS tmp;
	//Using this as a bad return
	tmp.shipType.onGrid[0].X = -1;
	//Get 3 integers from user -> d - direction, x and y coordinates
	cin >> d >> x >> y;
	if (d != 0 && d != 1) {
		cout << "Incorrect DIRECTION value. Please enter 0 or 1 to specifiy the orientation of the ship.\n";
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		return tmp;
	}
	
	if (x < 0 || x >= BOARD_WIDTH) return tmp;
	if (y < 0 || y >= BOARD_HEIGHT) return tmp;
	//Good data
	tmp.direction = (DIRECTION)d;
	tmp.shipType.onGrid[0].X = x;
	tmp.shipType.onGrid[0].Y = y;
	return tmp;
}

void LoadShips()
{
	//Sets the default data for the ships
	ship[0].name = "Scout Ship"; ship[0].length = 2;
	ship[1].name = "Artillery Ship"; ship[1].length = 3;
	ship[2].name = "The Destroyer"; ship[2].length = 4;
	ship[3].name = "Aircraft Carrier"; ship[3].length = 5;
}
void ResetBoard()
{
	//Loop through each player
	for (int playerIndex = 1; playerIndex < 3; ++playerIndex)
	{
		//For each grid point, set contents to 'water'
		for (int w = 0; w <= BOARD_WIDTH; w++) {
			for (int h = 0; h <= BOARD_HEIGHT; h++) {
				player[playerIndex].grid[w][h] = WATER;
			}
		}
		//Loop back to next player -> playerIndex 1, 2
	}
}


void DrawBoard(int thisPlayer)
{
	//Array that contains the letters
	char gridLetters[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T' };

	//Draws the board for a player (thisPlayer)
	cout << "PLAYER " << thisPlayer << "'s BOARD\n";
	cout << "----------------------\n";

	//Loop through the board with and draw the numbers
	cout << "   "; //left-padding for number row on top
	for (int w = 0; w < BOARD_WIDTH; w++) {
		if (w < BOARD_WIDTH)
			cout << w << "  ";
	};

	cout << "\n";

	//Loop through each grid point and display to console
	for (int h = 0; h < BOARD_HEIGHT; h++) {
		for (int w = 0; w < BOARD_WIDTH; w++) {
			//Displaying the letters

			if (w == 0) {
				cout << gridLetters[h] << "  ";
			}
			if (w > 10) {
				cout << " ";
			}
			//Display contents of this grid -> when game not running we are still placing ships
			if (guessingStage == false) cout << player[thisPlayer].grid[w][h] << "  ";

			/*
			if (guessingStage == true && player[thisPlayer].grid[0][0] == HIT) {
				player[thisPlayer].grid[0][0] = 'x';
			}
			else if (guessingStage == true && player[thisPlayer].grid[0][0] == MISS) {
				player[thisPlayer].grid[0][0] = 'o';
			}
			*/

			if (guessingStage == true && player[thisPlayer].grid[w][h] != is_SHIP)
			{
				cout << player[thisPlayer].grid[w][h] << "  ";
			}
			else if (guessingStage == true && player[thisPlayer].grid[w][h] == is_SHIP)
			{
				cout << WATER << "  ";
			}
			if (w == BOARD_WIDTH - 1) cout << endl;
		}
	}
}
