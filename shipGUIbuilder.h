#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

Mat combineImage(Mat image1, Mat image2, int xStart, int yStart);


class gridDisplayer {

private:
	Mat gridDisplay;
	int x;
	int y;
public:

	gridDisplayer(Mat grid, int locationX, int locationY, bool rotated) { //displayer constructor. Call it everytime a ship needs to be displayed, use required parameters. 
		x = locationX;
		y = locationY;
		gridDisplay = grid;

	}

	void display(Mat& background) {
		Mat display = combineImage(background, gridDisplay, x, y);
		//return display;
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
public:

	shipDisplayer(Mat background, Mat ship, int locationX, int locationY, bool rotated) { //displayer constructor. Call it everytime a ship needs to be displayed, use required parameters. 


		if (rotated == false) {
			shipDisplay = ship;
			x = (background.cols / 20) * locationX + (((background.cols) / 40)) - (ship.cols / 2);
			y = (background.rows / 10) * locationY + ((background.rows) / 20) - (ship.rows / 2);
		}
		else {
			rotate(ship, ship, ROTATE_90_CLOCKWISE);
			shipDisplay = ship;
			x = (background.cols / 20) * locationX + (ship.rows / 2);
			y = (background.rows / 10) * locationY + ((background.rows) / 20) - (ship.rows / 2);
		}
	}

	Mat display(Mat& background) {
		Mat display = combineImage(background, shipDisplay, x, y);
		return display;
	}


};

Mat combineImage(Mat image1, Mat image2, int xStart, int yStart) {
	int h = image2.rows;
	int w = image2.cols;

	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			Vec3b bgr = image2.at<Vec3b>(y, x);
			if (!(bgr[0] == 0 & bgr[1] == 0 & bgr[2] == 0)) {
				image1.at<Vec3b>(y + yStart, x + xStart) = image2.at<Vec3b>(y, x);
			}
		}
	}

	return image1;
}