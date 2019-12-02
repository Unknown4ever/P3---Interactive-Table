#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <chrono>

using namespace cv;
using namespace std;
using namespace std::chrono_literals;


class gridDisplayer {

private:
	Mat gridDisplay;
	int x;
	int y;
	int shotStatus;
	Mat hitDot;
	Mat missDot;
	Mat hasBoatSQ;
	Mat noBoatSQ;

	void scoutColouring(bool isBoat) {
		if (isBoat == false) {
			gridDisplay = combineImage(gridDisplay, noBoatSQ, 10, 10);
		}
		else {
			gridDisplay = combineImage(gridDisplay, hasBoatSQ, 10, 10);
		}
		this_thread::sleep_for(3s);
		gridDisplay = imread("Ships/grid.png");
		shoot(shotStatus);
		//terminate();
	}

public:
	//displayer constructor. Call it everytime a ship needs to be displayed, use required parameters. 
	gridDisplayer(int locationX, int locationY, bool rotated) { 
		x = locationX;
		y = locationY;
		gridDisplay = imread("Ships/grid.png");
		shotStatus = 0;
		hitDot = imread("Ships/gridHitDot.png");
		missDot = imread("Ships/gridMissDot.png");
		hasBoatSQ = imread("Ships/gridScoutDark.png");
		noBoatSQ = imread("Ships/gridScoutBright.png");
	}


	void shoot(int status) {
		if (status == 1) {
			gridDisplay = combineImage(gridDisplay, hitDot, 50, 10);
		}
		else if (status == 2) {
			gridDisplay = combineImage(gridDisplay, missDot, 50, 10);
		}
		shotStatus = status;
	}


	void scout(bool isBoat) {
		//scoutColouring(isBoat);
		thread t(&gridDisplayer::scoutColouring, this, isBoat);
		t.detach();
	}


	void display(Mat& background) {
		shoot(shotStatus);
		combineImage(background, gridDisplay, x, y);
	}

};

class shipDisplayer {
private:
	int x;
	int y;
	Mat shipDisplay;
	bool rotated;
	Mat shipRotated;
	int locationX;
	int locationY;
	Mat background;

	void identifyShip(int size) {
		switch (size)
		{
		case(1): 
			shipDisplay = imread("Ships/decoy.png");
			resize(shipDisplay, shipDisplay, Size(), 0.4, 0.3);
			break;
		case(2):
			shipDisplay = imread("Ships/scout.png");
			resize(shipDisplay, shipDisplay, Size(), 1, 1.1);
			break;
		case(3):
			shipDisplay = imread("Ships/battleship.png");
			resize(shipDisplay, shipDisplay, Size(), 1, 0.9);
			break;
		case(4):
			shipDisplay = imread("Ships/submarine.png");
			resize(shipDisplay, shipDisplay, Size(), 1, 0.9);
			break;
		case(5):
			shipDisplay = imread("Ships/carrier.png");
			resize(shipDisplay, shipDisplay, Size(), 0.75, 0.75);
			break;
		default:
			break;
		}
	}
public:
	//displayer constructor. Call it everytime a ship needs to be displayed, use required parameters. 
	shipDisplayer(Mat background, int size, int locationX, int locationY, bool rotated) { 
		identifyShip(size);
		if (rotated == false) {
			x = (background.cols / 20) * locationX + (((background.cols) / 40)) - (shipDisplay.cols / 2);
			y = (background.rows / 10) * locationY + 10;
		}
		else {
			rotate(shipDisplay, shipDisplay, ROTATE_90_CLOCKWISE);
			x = (background.cols / 20) * locationX + 10;
			y = (background.rows / 10) * locationY + ((background.rows) / 20) - (shipDisplay.rows / 2);
		}
		
	}

	void display(Mat bg) {
		Mat display = combineImage(bg, shipDisplay, x, y);
	}
};

class GridView {
private:
	std::vector<std::vector<gridDisplayer>> gridVector;
public:
	GridView(Mat background) {
		for (int countX = 0; countX <= background.cols - 40; countX += 80) {
			std::vector<gridDisplayer> localDisplayer;
			for (int countY = 0; countY <= background.rows - 40; countY += 80) {
				localDisplayer.push_back(gridDisplayer(countX, countY, false));
			}
			gridVector.push_back(localDisplayer);
		}
	}
	

	void shoot(int status, int x, int y) {
		if (status > 0 & status < 3) {
			gridVector.at(x).at(y).shoot(status);
		}
	}

	void scout(int x, int y, bool isBoat) {
		gridVector.at(x).at(y).scout(isBoat);
	}

	void display(Mat& background) {
		for (int x = 0; x < gridVector.size(); x++) {
			for (int y = 0; y < gridVector.at(x).size(); y++) {
				gridVector.at(x).at(y).display(background);
			}
		}
	}
};

class PlayerGrid {
private:
	Mat background;
	Mat displayground;
	GridView grid = GridView(displayground);
	vector<shipDisplayer> boats;
	String name;

public:
	PlayerGrid(String n) {
		Mat bg = imread("Ships/background.png");
		bg.copyTo(background);
		background.copyTo(displayground);
		grid = GridView(displayground);
		name = n;
	}

	void display() {
		background.copyTo(displayground);
		grid.display(displayground);
		imshow(name, displayground);
	}

	void displayWithBoats() {
		display();
		for (int i = 0; i < boats.size(); i++) {
			boats.at(i).display(displayground);
		}
		imshow(name, displayground);
	}

	void shoot(int status, int x, int y) {
		grid.shoot(status, x, y);
	}

	void scout(bool status, int x, int y) {
		grid.scout(x, y, status);
	}

	void addBoat(int size, int locationX, int locationY, bool rotated) {
		boats.push_back(shipDisplayer(displayground, size, locationX, locationY, rotated));
	}
};

void resizeBackground(Mat& background, int x, int y) {
	double factorX = 0.0;
	double factorY = 0.0;
	factorX = double(x) / double(background.cols);
	factorY = double(y) / double(background.rows);
	resize(background, background, Size(), factorX, factorY);
}

