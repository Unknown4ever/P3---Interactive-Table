#pragma once
#include <opencv2\opencv.hpp>
#include "ImageShipDetector.h"

using namespace cv;
using namespace std;

class Gridfinder {
private:
	Rect grid;

public:
	void cropToGrid(Mat& image) {
		if (!grid.empty()) {
			image = image(grid);
		}
	}
	void findGrid(Mat image) {
		Mat mask;
		image.copyTo(mask);
		// Blue
		int upLim[] = { 140,200,200 };
		int lowLim[] = { 80,50,100 };
		mask = makeMask(mask, lowLim, upLim);
		vector<vector<Point>> contours;
		contours = shapeDetection(mask, image);
		vector<Point> gridPoints;
		for (int i = 0; i < contours.size(); i++) {
			for (int j = 0; j < contours.at(i).size(); j++) {
				gridPoints.push_back(contours.at(i).at(j));
			}
		}
		grid = boundingRect(gridPoints);
	}
};