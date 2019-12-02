#pragma once
#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;

/*
To get the information for the boat, the following code can be copied into the main code
vector<vector<int>> boats;
boats = scanForBoats(image);
*/
Mat makeMask(Mat image) {
	Mat mask;

	int upLim[] = { 90,240,240 };
	int lowLim[] = { 40,50,0 };

	if (image.empty()) {
		//cout << "Could no find image" << endl;
	}
	else {
		cvtColor(image, image, COLOR_BGR2HSV);
		image.copyTo(mask);
		inRange(mask, Scalar(lowLim[0], lowLim[1], lowLim[2]), Scalar(upLim[0], upLim[1], upLim[2]), mask);

		erode(mask, mask, Mat(), Point(-1, -1), 3);
		dilate(mask, mask, Mat(), Point(-1, -1), 4);
	}
	return mask;
}

void setLabel(Mat& im, const string label, vector<Point>& contour) {
	int fontface = FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	Size text = getTextSize(label, fontface, scale, thickness, &baseline);
	Rect r = boundingRect(contour);

	Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	rectangle(im, pt + Point(0, baseline), pt + Point(text.width, -text.height), CV_RGB(255, 255, 255), FILLED);
	putText(im, label, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}

vector<vector<int>> findBoats(vector<vector<Point>> contours, Mat image) {
	vector<vector<int>> boats;
	for (int i = 0; i < contours.size(); i += 2) {
		vector<Point> contour;
		contour = contours.at(i);
		Rect bBox = boundingRect(contour);
		float height = float(bBox.height);
		float width = float(bBox.width);
		double ratio = 0.00;
		//Part that find the direction of the ship
		bool direction;
		//Ship is vertical
		if (width < height) {
			ratio = double(height / width);
			direction = true;
		}
		//Ship is horisontal
		else {
			ratio = double(width / height);
			direction = false;
		}
		//cout << "Width: " << width << " Height: " << height << " Ratio: " << ratio << endl;

		//Part that finds the size of the ship
		vector<vector<double>> boatRatios;
		boatRatios.push_back(vector<double>{ 2, 0 });
		boatRatios.push_back(vector<double>{ 3, 2.5 });
		boatRatios.push_back(vector<double>{ 4, 3.5 });
		boatRatios.push_back(vector<double>{ 5, 5.5 });

		double bestFit = 10;
		int boatSize;

		for (int j = 0; j < boatRatios.size(); j++) {
			if (ratio > boatRatios.at(j).at(1)) {
				boatSize = boatRatios.at(j).at(0);
			}
		}
		//Part that finds the position of the ship
		vector<int> vlines;
		vector<int> hlines;
		for (int k = 0; k < 12; k++) {
			hlines.push_back((image.rows / 12)*k);
		}
		for (int k = 0; k < 22; k++) {
			vlines.push_back((image.cols / 22)*k);
		}

		int x = 0;
		int y = 0;
		//Find x and y
		//Vertical
		if (direction == true) {
			//Find x
			for (int j = 0; j < vlines.size() - 1; j++) {
				if (vlines.at(j) < bBox.x & bBox.x > vlines.at(j + 1)) {
					x = j;
				}
			}
			//Find y
			for (int j = 0; j < hlines.size() - 1; j++) {
				if (hlines.at(j) < bBox.y + height*0.4 & bBox.y + height*0.4 > hlines.at(j + 1)) {
					y = j;
				}
			}
		}
		//Horisontal
		else {
			//Find x
			for (int j = 0; j < vlines.size() - 1; j++) {
				if (vlines.at(j) < bBox.x + height*0.4 & bBox.x + height*0.4 > vlines.at(j + 1)) {
					x = j;
				}
			}
			//Find y
			for (int j = 0; j < hlines.size() - 1; j++) {
				if (hlines.at(j) < bBox.y & bBox.y > hlines.at(j + 1)) {
					y = j;
				}
			}
		}

		vector<int> boat;
		boat.push_back(x);
		boat.push_back(y);
		boat.push_back(boatSize);
		boat.push_back(direction);

		boats.push_back(boat);
	}
	return boats;
}

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


vector<vector<Point>> shapeDetection(Mat mask, Mat image) {
	Mat canny_output;
	image.copyTo(canny_output);
	int thresh = 80;
	int max_thresh = 255;

	vector<vector<Point> > contours;
	vector<Point> approx;
	vector<Vec4i> hierarchy;

	Canny(mask, canny_output, thresh, thresh * 2, 3);
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	/*
	Mat dst;
	canny_output.copyTo(dst);

	for (int i = 0; i < contours.size(); i++) {
		approxPolyDP(Mat(contours.at(i)), approx, arcLength(Mat(contours.at(i)), true)*0.02, true);
		if (fabs(contourArea(contours.at(i))) < 100 || !isContourConvex(approx)) {
			continue;
		}
		int vtc = approx.size();
		setLabel(dst, to_string(vtc), contours.at(i));
	}
	cvtColor(dst, dst, COLOR_BayerBG2BGR);
	dst = combineImage(image, dst, 0, 0);
	*/
	return contours;
}

//Debug function
void printBoats(vector<vector<int>> boats) {
	cout << "New batch of boats:" << endl;
	for (int i = 0; i < boats.size(); i++) {
		vector<int> boat = boats.at(i);
		cout << "A boat: " << endl;
		cout << "x: " << boat.at(0) << ", y: " << boat.at(1) << ", size: " << boat.at(2) << ", direction: " << boat.at(3) << endl;
	}
}

//Main function for returning vector for boats
vector<vector<int>> scanForBoats(Mat image) {
	Mat mask;
	image.copyTo(mask);
	mask = makeMask(mask);
	vector<vector<int>> boats;
	vector<vector<Point>> contours;
	contours = shapeDetection(mask, image);
	boats = findBoats(contours, image);
	return boats;
}
