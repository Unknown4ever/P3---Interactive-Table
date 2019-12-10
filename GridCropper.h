#pragma once
#include <opencv2\opencv.hpp>
#include "ImageShipDetector.h"
#include <algorithm>

using namespace cv;
using namespace std;

class Gridfinder {
private:
	Rect grid;
	Point top_left;
	Point butt_left;
	Point top_right;
	Point butt_right;
	double top_a;
	double butt_a;
	double left_a;
	double right_a;

	int displacementY;
	int displacementX;

	struct gridCorner{
		Rect bBox;
		Point cornerPoint;
	};

	bool compareX(gridCorner a, gridCorner b) {
		return a.bBox.x > b.bBox.x;
	}

	int roundoff(int number, int uplim, int lowlim) {
		if (number > uplim) number = uplim;
		if (number < lowlim) number = lowlim;
		return number;
	}

	void sortX(vector<gridCorner> &boxes) {
		vector<gridCorner> sortedBoxes;
		sortedBoxes.push_back(boxes.at(0));
		for (int i = 1; i < boxes.size(); i++) {
			bool placed = false;
			
			for (int j = 0; j < sortedBoxes.size(); j++){
				if (boxes.at(i).bBox.x < sortedBoxes.at(j).bBox.x & placed != true) {
					sortedBoxes.insert(sortedBoxes.begin() + j, boxes.at(i));
					placed = true;
				}
			}

			if (placed == false) {
				sortedBoxes.push_back(boxes.at(i));
			}
		}

		boxes = sortedBoxes;

		if (boxes.at(0).bBox.y > boxes.at(1).bBox.y) {
			gridCorner con = boxes.at(0);
			boxes.at(0) = boxes.at(1);
			boxes.at(1) = con;
		}
		if (boxes.at(2).bBox.y > boxes.at(3).bBox.y) {
			gridCorner con = boxes.at(2);
			boxes.at(2) = boxes.at(3);
			boxes.at(3) = con;
		}
	}

public:
	void cropToGrid(Mat& image) {
		if (!grid.empty()) {
			Mat rescaledimage;
			image = image(grid);
			image.copyTo(rescaledimage);
			imshow("Gridcropper: Pre-scaled image", image);
			waitKey(1);
			
			//Scale for y
			for (int x = 0; x < rescaledimage.cols; x++)
			{
				int imageHeight = (butt_a*x + butt_left.y) - (top_a*x + top_left.y);
				//cout << "Image Height: " << imageHeight << endl;
				double scaler = double(imageHeight) / double(rescaledimage.rows);

				for (int y = 0; y < rescaledimage.rows; y++)
				{
					//cout << "scaling at: " << x << ", " << y << endl;
					//cout << roundoff(int(scaler*y) + int(top_a*x), image.rows-1, 0) << ", " << roundoff(x, image.cols-1, 0) << endl;
					rescaledimage.at<Vec3b>(y, x) =
						//image.at<Vec3b>(roundoff(int(scaler*y), image.rows-1, 0), roundoff(x, image.cols-1, 0));
						image.at<Vec3b>(roundoff(int(scaler*y) + int(top_a*x) /*- displacementY*/, image.rows - 1, 0), x);
				}
			}
			rescaledimage.copyTo(image);
			
			//Scale for x
			for (int y = 0; y < rescaledimage.rows; y++)
			{
				int imageHeight = (right_a*y + top_right.x) - (left_a*y + top_left.x);
				//cout << "Image Height: " << imageHeight << endl;
				double scaler = double(imageHeight) / double(rescaledimage.cols);

				for (int x = 0; x < rescaledimage.cols; x++)
				{
					//cout << "scaling at: " << x << ", " << y << endl;
					//cout << roundoff(int(scaler*y) + int(top_a*x), image.rows-1, 0) << ", " << roundoff(x, image.cols-1, 0) << endl;
					rescaledimage.at<Vec3b>(y, x) =
						//image.at<Vec3b>(roundoff(int(scaler*y), image.rows-1, 0), roundoff(x, image.cols-1, 0));
						image.at<Vec3b>(y, roundoff(int(scaler*x) + int(left_a*y) + displacementX, image.cols - 1, 0));
				}
			}
			image = rescaledimage;
		}
	}


	void findGrid(Mat image) {
		imshow("Gridcropper: image to scan for grid", image);
		Mat mask;
		image.copyTo(mask);
		// Blue
		int upLim[] = { 120,255,255 };
		int lowLim[] = { 100,100,150 };
		mask = makeMask(mask, lowLim, upLim);
		erode(mask, mask, Mat(), Point(-1, -1), 1);
		dilate(mask, mask, Mat(), Point(-1, -1), 2);
		imshow("Gridcropper: mask", mask);
		//waitKey(0);
		vector<vector<Point>> contours;
		contours = edgeDetection(mask, image);

		vector<vector<Point>> new_contours;
		for (int i = 0; i < contours.size(); i++) {
			vector<Point> approx;
			approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true)*0.02, true);
			if (!(fabs(contourArea(contours.at(i))) < 100 /* || !isContourConvex(approx) */)) {
				cout << "GC: Size matters: " << fabs(contourArea(contours.at(i))) << 20 << endl;
				new_contours.push_back(contours.at(i));
			}
		}
		contours = new_contours;

		vector<gridCorner> cornerBoxes;
		for (int i = 0; i < contours.size(); i += 2) {
			gridCorner corner;
			corner.bBox = boundingRect(contours.at(i));
			cornerBoxes.push_back(corner);
			cout << "Corner: " << corner.bBox.x << ", " << corner.bBox.y << endl;
		}

		if (cornerBoxes.size()>3) {
			sortX(cornerBoxes);
			top_left = Point(cornerBoxes.at(0).bBox.x, cornerBoxes.at(0).bBox.y);
			butt_left = Point(cornerBoxes.at(1).bBox.x, cornerBoxes.at(1).bBox.y+cornerBoxes.at(1).bBox.height);
			top_right = Point(cornerBoxes.at(2).bBox.x + cornerBoxes.at(2).bBox.width, cornerBoxes.at(2).bBox.y);
			butt_right = Point(cornerBoxes.at(3).bBox.x + cornerBoxes.at(3).bBox.width, cornerBoxes.at(3).bBox.y + cornerBoxes.at(3).bBox.height);

			top_a = (double(top_right.y) - double(top_left.y)) / (double(top_right.x) - double(top_left.x));
			butt_a = (double(butt_right.y) - double(butt_left.y)) / (double(butt_right.x) - double(butt_left.x));

			left_a = (double(butt_left.x) - double(top_left.x)) / (double(butt_left.y) - double(top_left.y));
			right_a = (double(butt_right.x) - double(top_right.x)) / (double(butt_right.y) - double(top_right.y));

			displacementX = abs(top_left.x - butt_left.x);
			displacementY = abs(top_left.y - top_right.y);

			cout << "displacements: " << displacementX << ", " << displacementY << endl;

			vector<Point> otl;
			otl.push_back(top_left);
			otl.push_back(top_right);
			otl.push_back(butt_left);
			otl.push_back(butt_right);

			grid = boundingRect(otl);

			cout << "top_a: " << top_a << ", butt_a: " << butt_a << endl;

			for (int i = 0; i < cornerBoxes.size(); i++) {
				cout << "corner x: " << cornerBoxes.at(i).bBox.x << ", Corner y: " << cornerBoxes.at(i).bBox.y << endl;
			}
		}

		else if(!cornerBoxes.empty()) {
			vector<Point> gridPoints;
			for (int i = 0; i < contours.size(); i++) {
				for (int j = 0; j < contours.at(i).size(); j++) {
					gridPoints.push_back(contours.at(i).at(j));
				}
			}
			grid = boundingRect(gridPoints);
		}

		
		
		//grid = boundingRect(gridPoints);
	}
};