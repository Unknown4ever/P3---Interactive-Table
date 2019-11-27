#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "shipGUIbuilder.h"

using namespace cv;
using namespace std;

Mat background = imread("background.png");
Mat carrier = imread("carrier.png");
Mat battleship = imread("battleship.png"); // all ship sprites loaded
Mat submarine = imread("submarine.png");
Mat scout = imread("scout.png");
Mat grid = imread("grid.png");

Mat combineImage(Mat image1, Mat image2, int xStart, int yStart);

int main() {
	// to display a new ship, write 'Displayer <name>(<ship class>, <X coord>, <Y coord>, <boolean for rotation. True = 90 degree rotation, False = null rotation>);
	// then do <name>.display();

	std::vector<std::vector<gridDisplayer>> gridVector;

	for (int countX = 0; countX <= background.cols-40; countX+=80) {
		std::vector<gridDisplayer> localDisplayer;
		for(int countY = 0; countY <= background.rows-40; countY+=80){
			localDisplayer.push_back( gridDisplayer (grid, countX, countY, false));
		}
		gridVector.push_back(localDisplayer);
	}

	for (int x = 0; x < gridVector.size(); x++) {
		for (int y = 0; y < gridVector.at(x).size(); y++) {
			gridVector.at(x).at(y).display(background);
		}
	}
	


	shipDisplayer USS_Obama(background, carrier, 10, 3, true);
	shipDisplayer USS_Harling(background, battleship, 10, 5, false);
	shipDisplayer Seagul(background, scout, 4, 5, true);
	shipDisplayer USS_Texas(background, submarine, 8, 7, false);
	USS_Obama.display(background);
	USS_Harling.display(background);
	USS_Texas.display(background);
	Seagul.display(background);

	double factorX = 0.0;
	double factorY = 0.0;
	factorX = double(1920) / double(background.cols);
	factorY = double(1080) / double(background.rows);
	resize(background, background, Size(), factorX, factorY);
	imshow("output", background);

	waitKey(0);
	return 0;
}


