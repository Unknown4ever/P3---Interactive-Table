#include <iostream>
#include <thread>
#include <chrono>
#include <string> 
#include <dos.h> //for delay
#include <conio.h> //for getch()
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "ImageShipDetector.h"
#include "ImageShotDetector.h"
#include "shipGUIbuilder.h"
#include "GridCropper.h"

using namespace std;
using namespace cv;
using namespace std::chrono;

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
bool longStick = false;
bool regularShot = false;

bool inputGo = false;
bool showBoatsInGame = false;

VideoCapture cap(1);


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
	PlayerGrid playerGrid = PlayerGrid("Battleship");
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
Gridfinder gridfinder;


//Functions
void LoadShips();
void DrawBoard(int);
void inputTracker();
void liveFeeder();
PLACESHIPS UserInputShipPlacement(vector<int>);
bool UserInputAttack(int& x, int& y, int& d, int& type, int);
bool GameOverCheck(int);




int main()
{
	thread t(inputTracker);
	t.detach();
	//thread t2(liveFeeder);
	//t2.detach();
	system("cls");
	cout << "Ajust Camera and type 'go' when ready" << endl;
	while (inputGo == false) {
		Mat image;
		cap >> image;
		imshow("Livefeed", image);
		waitKey(1);
		player[0].playerGrid.display();
	}
	cv::destroyWindow("Livefeed");
	inputGo = false;

	Mat imagegrid;
	cap >> imagegrid;
	gridfinder.findGrid(imagegrid);

	LoadShips();

	//insert function here in the future for changing between abilities
	regularShot = false;
	multiShot = false;
	superMultiShot = false;
	strafeRun = false;
	longStick = false;
	scoutArea = false;

	//displayImage("grid.png");
	//displayImage("carrier.png");
	//Loop through each player and place ships
	for (int aPlayer = 1; aPlayer < 3; aPlayer++)
	{
		player[aPlayer].playerGrid.sendMessage("Player " + to_string(aPlayer) + ", place ships!");

		player[aPlayer].playerGrid.display();
		cv::waitKey(1);

		auto start = high_resolution_clock::now();
		vector<vector<int>> boats;
		system("cls");
		cout << "Player " << aPlayer << ", Place boats!" << endl << "Type 'go' to go to next step" << endl;
		while (inputGo == false)
		{
			//cout << time_point_cast<milliseconds>(start) << endl;
			PlayerGrid grid("Battleship");
			Mat image;
			cap >> image;
			//gridfinder.findGrid(image);
			gridfinder.cropToGrid(image);

			boats = scanForBoats(image);
			for (int i = 0; i < boats.size(); i++)
			{
				grid.addBoat(boats.at(i).at(2), boats.at(i).at(0), boats.at(i).at(1), !(bool(boats.at(i).at(3))));
			}
			grid.displayWithBoats();
			cv::waitKey(1);
		}
		cv::destroyWindow("main132");
		inputGo = false;

		player[aPlayer].playerGrid.display();
		cv::waitKey(10);

		Mat image;
		cap >> image;
		//gridfinder.findGrid(image);
		gridfinder.cropToGrid(image);
		boats = scanForBoats(image);

		//Loop through each ship type to place
		for (int i = 0; i < boats.size(); i++)
		{
			//cout << "placing boat of size: " << boats.at(i).at(2) << endl;
			//this_thread::sleep_for(3s);
			int this_SHIP = boats.at(i).at(2) - 2;

			//Display board
			//gridfinder.cropToGrid(image);
			std::system("cls");
			DrawBoard(aPlayer); //Displaying the board for ship placement stage

								//Get input from user and loop until good data is returned
			PLACESHIPS aShip;
			aShip.shipType.onGrid[0].X = -1;

			aShip = UserInputShipPlacement(boats.at(i));
			//std::cout << "boat size is: " << aShip.shipType.length << endl;
			if (aShip.shipType.onGrid[0].X != -1 && boats.at(i).at(2) > 0)
			{
				//Combine user data with "this ship" data
				aShip.shipType.length = ship[this_SHIP].length;
				aShip.shipType.name = ship[this_SHIP].name;

				player[aPlayer].playerGrid.addBoat(ship[this_SHIP].length, aShip.shipType.onGrid[0].X, aShip.shipType.onGrid[0].Y, aShip.direction);
				player[aPlayer].playerGrid.displayWithBoats();
				cv::waitKey(1);

				//Add the FIRST grid point to the current player's game board
				player[aPlayer].grid[aShip.shipType.onGrid[0].X][aShip.shipType.onGrid[0].Y] = is_SHIP;

				//Determine ALL grid points based on length and direction
				for (int i = 1; i < aShip.shipType.length; ++i)
				{
					if (aShip.direction == HORIZONTAL) {
						aShip.shipType.onGrid[i].X = aShip.shipType.onGrid[i - 1].X;
						aShip.shipType.onGrid[i].Y = aShip.shipType.onGrid[i - 1].Y + 1;
					}
					if (aShip.direction == VERTICAL) {
						aShip.shipType.onGrid[i].Y = aShip.shipType.onGrid[i - 1].Y;
						aShip.shipType.onGrid[i].X = aShip.shipType.onGrid[i - 1].X + 1;
					}

					//Add the REMAINING grid points to our current players game board
					player[aPlayer].grid[aShip.shipType.onGrid[i].X][aShip.shipType.onGrid[i].Y] = is_SHIP;

				}
				//Loop back through each ship type
			}
		}
		//Loop back through each player
	}

	//Game part after ships have been placed by both players
	guessingStage = true;
	int thisPlayer = 1;
	do {

		inputGo = false;
		//Because we are ATTACKING now, the opposite players board is the display board
		int enemyPlayer;
		if (thisPlayer == 1) enemyPlayer = 2;
		if (thisPlayer == 2) enemyPlayer = 1;
		std::system("cls");
		DrawBoard(enemyPlayer);
		player[enemyPlayer].playerGrid.sendMessage("Player " + to_string(thisPlayer) + ", shoot!");
		player[enemyPlayer].playerGrid.display();
		cv::waitKey(1);
		cout << "Player " << thisPlayer << ", Shoot!" << endl << "Type 'go' to continue to go to next step" << endl;
		while (inputGo == false)
		{
			vector<int> shot;
			shot = scanForShot(cap, gridfinder);
			if (!shot.empty()) {
				player[enemyPlayer].playerGrid.preShoot(shot.at(2), shot.at(0), shot.at(1));
				if (showBoatsInGame == false) player[enemyPlayer].playerGrid.display();
				else player[enemyPlayer].playerGrid.displayWithBoats();
			}
			waitKey(1);
		}
		//Get attack coords from this player
		bool goodInput = false;
		int x, y, shotType, d;
		while (goodInput == false) {
			goodInput = UserInputAttack(x, y, d, shotType, thisPlayer);
		}
		regularShot = false;
		multiShot = false;
		superMultiShot = false;
		strafeRun = false;
		longStick = !bool(d);
		scoutArea = false;

		if (shotType == 1)
		{
			regularShot = true;
		}
		else if (shotType == 2)
		{
			scoutArea = true;
		}
		else if (shotType == 3)
		{
			multiShot = true;
		}
		else if (shotType == 4)
		{
			superMultiShot = true;
		}
		else if (shotType == 5)
		{
			strafeRun = true;
		}


		// ABILITIES //

		//Regular shot

		if (regularShot == true) {

			if (player[enemyPlayer].grid[x][y] == is_SHIP) {
				player[enemyPlayer].grid[x][y] = HIT;
				player[enemyPlayer].playerGrid.shoot(1, x, y);
				std::system("cls");
				DrawBoard(enemyPlayer);
				std::cout << "BOOM";
			}
			else
			{
				std::cout << "Miss" << endl;
				player[enemyPlayer].playerGrid.shoot(2, x, y);
			}
			player[enemyPlayer].playerGrid.display();
			cv::waitKey(1);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		//Scout the enemy board and displays the ships for a few seconds
		//Will be true when the correct shape is detected by the camera

		if (scoutArea == true)
		{
			for (int x2 = -2; x2 < 3; x2++)
			{
				for (int y2 = -2; y2 < 3; y2++)
				{
					if (player[enemyPlayer].grid[x + x2][y + y2] == is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y + y2] = is_SHIP;
						if (!(x + x2 > 19 || x + x2 < 0 || y + y2 > 9 || y + y2 < 0))
						{
							player[enemyPlayer].playerGrid.scout(true, x + x2, y + y2);
						}

						std::cout << "ENEMY SPOTTED" << endl;
					}
					else if (player[enemyPlayer].grid[x + x2][y + y2] != is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y + y2] == MISS;
						if (!(x + x2 > 19 || x + x2 < 0 || y + y2 > 9 || y + y2 < 0))
						{
							player[enemyPlayer].playerGrid.scout(false, x + x2, y + y2);
						}
						std::cout << "YOU SUCK" << endl;
					}
				}
			}
			player[enemyPlayer].playerGrid.display();
			cv::waitKey(1);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		// Multishot

		if (multiShot == true) {
			for (int i = -1; i < 2; i++)
			{
				for (int j = -1; j < 2; j++)
				{
					if (i == 0 || j == 0)
					{
						if (player[enemyPlayer].grid[x + i][y + j] == is_SHIP)
						{
							if (!(x + i > 19 || x + i < 0 || y + j > 9 || y + j < 0))
							{
								player[enemyPlayer].grid[x + i][y + j] == HIT;
								player[enemyPlayer].playerGrid.shoot(1, x + i, y + j);
								std::cout << "BOOM" << endl;
							}
						}
						else if (player[enemyPlayer].grid[x + i][y + j] != is_SHIP)
						{
							if (!(x + i > 19 || x + i < 0 || y + j > 9 || y + j < 0))
							{
								player[enemyPlayer].grid[x + i][y + j] == MISS;
								player[enemyPlayer].playerGrid.shoot(2, x + i, y + j);
								std::cout << "MISS" << endl;
							}
						}
					}
				}
			}
			player[enemyPlayer].playerGrid.display();
			cv::waitKey(1);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		//Shoots in a 3x3 square

		if (superMultiShot == true)
		{
			for (int x2 = -1; x2 < 2; x2++)
			{
				for (int y2 = -1; y2 < 2; y2++)
				{
					if (player[enemyPlayer].grid[x + x2][y + y2] == is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y + y2] = HIT;
						if (!(x + x2 > 19 || x + x2 < 0 || y + y2 > 9 || y + y2 < 0))
						{
							player[enemyPlayer].playerGrid.shoot(1, x + x2, y + y2);
						}
						std::cout << "BOOM" << endl;
					}
					else if (player[enemyPlayer].grid[x + x2][y + y2] != is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y + y2] == MISS;
						if (!(x + x2 > 19 || x + x2 < 0 || y + y2 > 9 || y + y2 < 0))
						{
							player[enemyPlayer].playerGrid.shoot(2, x + x2, y + y2);
						}
						std::cout << "Miss" << endl;
					}
				}
			}
			player[enemyPlayer].playerGrid.display();
			cv::waitKey(1);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		//Shoots in a line

		if (strafeRun == true) {
			// false = horizontal 
			// true = vertical
			for (int x2 = 0; x2 < 5; x2++)
			{
				if (longStick == false)
				{
					if (player[enemyPlayer].grid[x + x2][y] == is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y] = HIT;
						if (!(x + x2 > 19 || x + x2 < 0 || y > 9 || y < 0))
						{
							player[enemyPlayer].playerGrid.shoot(1, x + x2, y);
						}
					}
					else if (player[enemyPlayer].grid[x + x2][y] != is_SHIP)
					{
						player[enemyPlayer].grid[x + x2][y] = MISS;
						if (!(x + x2 > 19 || x + x2 < 0 || y > 9 || y < 0))
						{
							player[enemyPlayer].playerGrid.shoot(2, x + x2, y);
						}
					}

				}
				if (longStick == true)
				{
					if (player[enemyPlayer].grid[x][y + x2] == is_SHIP)
					{
						player[enemyPlayer].grid[x][y + x2] = HIT;
						if (!(x > 19 || x < 0 || y + x2 > 9 || y + x2 < 0))
						{
							player[enemyPlayer].playerGrid.shoot(1, x, y + x2);
						}
					}
					else if (player[enemyPlayer].grid[x][y + x2] != is_SHIP)
					{
						player[enemyPlayer].grid[x][y + x2] = MISS;
						if (!(x > 19 || x < 0 || y + x2 > 9 || y + x2 < 0))
						{
							player[enemyPlayer].playerGrid.shoot(2, x, y + x2);
						}
					}
				}
			}
			player[enemyPlayer].playerGrid.display();
			cv::waitKey(1);
			std::this_thread::sleep_for(std::chrono::seconds(2));
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

	std::system("cls");
	std::cout << "\n\nPLAYER " << thisPlayer << " WON\n\n\n\n";
	player[0].playerGrid.sendMessage("Player " + to_string(thisPlayer) + " Won!");
	player[0].playerGrid.display();
	waitKey(1);
	std::system("pause");
	return 0;
}

void inputTracker()
{
	while (true)
	{
		string gogo;
		cin >> gogo;
		if (gogo == "go") inputGo = true;
		if (gogo == "findgrid") {
			cout << "Finding grid" << endl;
			Mat image;
			cap >> image;
			gridfinder.findGrid(image);
		}
		if (gogo == "showboats") showBoatsInGame = !showBoatsInGame;
	}
}

void liveFeeder() {
	{
		Mat image;
		//VideoCapture cap;
		while (true) {
			cap >> image;
			imshow("Livefeed", image);
			waitKey(1);
		}
	}
}

bool GameOverCheck(int enemyPLAYER)
{
	bool winner = true;
	//Loop through enemy board
	for (int w = 0; w < BOARD_WIDTH; w++) {
		for (int h = 0; h < BOARD_HEIGHT; h++) {
			//If any ships remain, game is still not over
			if (player[enemyPLAYER].grid[w][h] == is_SHIP)
			{
				winner = false;
				return winner;
			}
		}
	}
	//To know when is the end of the game
	return winner;
}


bool UserInputAttack(int& x, int& y, int& d, int& type, int theplayer)
{
	bool goodInput = false;
	//cin >> x >> y;
	vector<int> shot;
	bool shotFound = false;
	while (shotFound == false)
	{
		Mat image;
		cap >> image;
		//gridfinder.findGrid(image);
		shot = scanForShot(cap, gridfinder);
		if (!shot.empty() && shot.at(2) != 0) {
			shotFound = true;
		}
	}
	x = shot.at(0);
	y = shot.at(1);
	type = shot.at(2);
	d = !bool(shot.at(3));
	if (x < 0 || x >= BOARD_WIDTH) return goodInput;
	if (y < 0 || y >= BOARD_HEIGHT) return goodInput;
	goodInput = true;
	//delay(500);

	return goodInput;
}

PLACESHIPS UserInputShipPlacement(vector<int> boat)
{
	int d, x, y;
	PLACESHIPS tmp;
	//Using this as a bad return
	tmp.shipType.onGrid[0].X = -1;
	//Get 3 integers from user -> d - direction, x and y coordinates
	//cin >> d >> x >> y;
	d = !bool(boat.at(3));
	x = boat.at(0);
	y = boat.at(1);

	if (d != 0 && d != 1) {
		std::cout << "Incorrect DIRECTION value. Please enter 0 or 1 to specifiy the orientation of the ship.\n";
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
	std::cout << "PLAYER " << thisPlayer << "'s BOARD\n";
	std::cout << "----------------------\n";

	//Loop through the board with and draw the numbers
	std::cout << "   "; //left-padding for number row on top
	for (int w = 0; w < BOARD_WIDTH; w++) {
		if (w < BOARD_WIDTH)
			std::cout << w << "  ";
	};

	std::cout << "\n";

	//Loop through each grid point and display to console
	for (int h = 0; h < BOARD_HEIGHT; h++) {
		for (int w = 0; w < BOARD_WIDTH; w++) {
			//Displaying the letters

			if (w == 0) {

				std::cout << gridLetters[h] << "  ";
			}
			if (w > 10) {
				std::cout << " ";
			}
			//Display contents of this grid -> when game not running we are still placing ships
			if (guessingStage == false) std::cout << player[thisPlayer].grid[w][h] << "  ";

			if (guessingStage == true && player[thisPlayer].grid[w][h] != is_SHIP)
			{
				//std::cout << player[thisPlayer].grid[w][h] << "  ";
			}
			else if (guessingStage == true && player[thisPlayer].grid[w][h] == is_SHIP)
			{
				//std::cout << WATER << "  ";
			}
			if (w == BOARD_WIDTH - 1) std::cout << endl;
		}
	}
}