#pragma once
#include "ImageShipDetector.h"
#include "GridCropper.h"


Mat makeFineMask(Mat image, int lowLim1[], int upLim1[]) {
	Mat mask;
	int upLim[] = { upLim1[0],upLim1[1],upLim1[2] };
	int lowLim[] = { lowLim1[0],lowLim1[1],lowLim1[2] };

	if (image.empty()) {
		//cout << "Could no find image" << endl;
	}
	else {
		cvtColor(image, image, COLOR_BGR2HSV);
		//imshow("hsv", image);
		image.copyTo(mask);
		inRange(mask, Scalar(lowLim[0], lowLim[1], lowLim[2]), Scalar(upLim[0], upLim[1], upLim[2]), mask);

		//erode(mask, mask, Mat(), Point(-1, -1), 1);
		//dilate(mask, mask, Mat(), Point(-1, -1), 1);

	}
	return mask;
}

vector <int> findShot(int edgeCount, vector<vector<Point>> contours, Mat image, VideoCapture cap, Gridfinder gridfinder) {
	Rect bBox = boundingRect(contours.at(0));
	int x = 0;
	int y = 0;
	int shotShape = 0;

	int circleCount = 0;
	bool foundShape = false;

	for (int i = 0; i < 10; i++) {
		Mat circleImage;
		cap >> circleImage;
		Mat scr_gray;
		Mat mask;
		gridfinder.cropToGrid(circleImage);
		circleImage.copyTo(mask);
		cvtColor(circleImage, scr_gray, COLOR_BGR2GRAY);
		cvtColor(mask, mask, COLOR_BGR2HSV);
		GaussianBlur(mask, mask, Size(9, 9), 2, 2);

		//Isolation of colour
		inRange(mask, Scalar(40, 50, 0), Scalar(90, 240, 240), mask);
		bitwise_and(scr_gray, mask, scr_gray);

		

		vector<Vec3f> circles;

		imshow("Circle images", scr_gray);

		//Thresholding definition of circle
		HoughCircles(scr_gray, circles, HOUGH_GRADIENT, 1, scr_gray.rows / 8, 14, 13, 10, 17);

		if (!(circles.empty())) {
			circleCount++;
		}
	}

	//Noting the circle
	if (circleCount > 4) {
		cout << "It a circle!" << endl;
		shotShape = 2;
		foundShape = true;
	}
	bool direction = false;

	cout << "I'm dis edgy: " << edgeCount << endl;

	if (foundShape == false & edgeCount == 4) {

		double ratio = 0.00;
		//Ship is vertical
		if (bBox.width < bBox.height) {
			ratio = double(bBox.height) / double(bBox.width);
			direction = true;
		}
		//Ship is horisontal
		else {
			ratio = double(bBox.width) / double(bBox.height);
			direction = false;
		}
		//Strafe run
		if (float(ratio) > 1.8) {
			cout << "it a strafe" << endl;
			shotShape = 5;
			foundShape = true;
		}
		//Regular shot
		else {
			cout << "it a square" << endl;
			shotShape = 1;
			foundShape = true;
		}
	}

	//Triangle shot aka Big bomb
	if (foundShape == false & edgeCount == 3) {
		cout << "it a triangle" << endl;
		shotShape = 4;
		foundShape = true;
		x = 1;
		y = 1;
	}

	//Plus shot aka cross shot
	if (foundShape == false & edgeCount > 4) {
		cout << "it a cross" << endl;
		shotShape = 3;
		foundShape = true;
		x = 1;
		y = 1;
	}

	//Part that finds the position of the shot
	vector<int> vlines;
	vector<int> hlines;
	for (int k = 0; k < 11; k++) {
		hlines.push_back((image.rows / 12)*k);
	}
	for (int k = 0; k < 21; k++) {
		vlines.push_back((image.cols / 22)*k);
	}


	//Find x and y
	//Find x
	for (int j = 0; j < vlines.size() - 1; j++) {
		if (vlines.at(j) < bBox.x & bBox.x > vlines.at(j + 1)) {
			x = j;
		}
	}
	//Find y
	for (int j = 0; j < hlines.size() - 1; j++) {
		if (hlines.at(j) < bBox.y & bBox.y > hlines.at(j + 1)) {
			y = j;
		}
	}

	vector<int> finalShot;
	finalShot.push_back(x);
	finalShot.push_back(y);
	finalShot.push_back(shotShape);
	finalShot.push_back(direction);

	return finalShot;
}

int shapeDetection(vector<vector<Point>> contours) {
	//Mat dst;
	//canny_output.copyTo(dst);
	int vtc = 0;
	vector<Point> approx;
	approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true)*0.02, true);
	if (!(fabs(contourArea(contours.at(0))) < 100 || !isContourConvex(approx))) {
		vtc = approx.size();
	}

	//setLabel(dst, to_string(vtc), contours.at(i));

	//cvtColor(dst, dst, COLOR_BayerBG2BGR);
	//dst = combineImage(image, dst, 0, 0);
	return vtc;
}

//Debug function
void printShot(vector<int> shot) {
	cout << "Shot: " << endl;
	cout << "x: " << shot.at(0) << ", y: " << shot.at(1) << ", type: " << shot.at(2) << ", direction: " << shot.at(3) << endl;
}

vector<int> scanForShot(VideoCapture cap, Gridfinder gridfinder) {
	Mat mask;
	Mat image;
	cap >> image;
	gridfinder.cropToGrid(image);
	image.copyTo(mask);
	int upLim[] = { 90,240,200 };
	int lowLim[] = { 40,50,0 };
	//int upLim[] = { 50,255,50 };
	//int lowLim[] = { 0,200,0 };
	GaussianBlur(mask, mask, Size(9, 9), 2, 2);

	mask = makeFineMask(mask, lowLim, upLim);
	dilate(mask, mask, Mat(), Point(-1, -1), 1);
	
	//mask = makeGreenMask(mask);
	vector<vector<Point>> contours;
	imshow("Le mask: ", mask);
	contours = edgeDetection(mask, image);
	vector<vector<Point>> new_contours;
	for (int i = 0; i < contours.size(); i++) {
		
		vector<Point> approx;
		approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true)*0.02, true);
		//cout << "area: " << fabs(contourArea(contours.at(0))) << endl;
		if (!(fabs(contourArea(contours.at(0))) < 5 /*|| !isContourConvex(approx)*/)) {
			new_contours.push_back(contours.at(0));
			//cout << "Contour: " << new_contours.at(0) << endl;
		}
	}
	
	contours = new_contours;

	vector<int> shot;
	if (!(contours.empty())) {
		int edgeCount = shapeDetection(contours);
		shot = findShot(edgeCount, contours, image, cap, gridfinder);
	}
	return shot;
}