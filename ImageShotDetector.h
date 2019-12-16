#pragma once
#include "ImageShipDetector.h"
#include "GridCropper.h"
#include <cmath>


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

vector <int> findShot(int edgeCount, vector<vector<Point>> contours, Mat image, VideoCapture cap, Gridfinder& gridfinder) {
	Rect bBox = boundingRect(contours.at(0));
	
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

		//Thresholding definition of circle
		HoughCircles(scr_gray, circles, HOUGH_GRADIENT, 1, scr_gray.rows / 8, 14, 17, 5, 17);
		

		if (!(circles.empty())) {
			circleCount++;
			Point center = Point(cvRound(circles.at(0)[0]), cvRound(circles.at(0)[1]));
			int radius = cvRound(circles.at(0)[2]);
			cout << "Center: " << center << "and radius: " << radius << endl;
			circle(scr_gray, center, radius, Scalar(255), 2);
			imshow("Circle images", scr_gray);
		}
	}

	//Noting the circle
	cout << "BBOX: " << bBox.height << endl; 
	if (circleCount > 6 & bBox.height < image.rows/9) {
		cout << "it a circle!" << endl;
		shotShape = 2;
		foundShape = true;
	}
	bool direction = false;

	Mat imageCopy;
	image.copyTo(imageCopy);
	//dilate(imageCopy, imageCopy, Mat(), Point(-1, -1), 6);
	//erode(imageCopy, imageCopy, Mat(), Point(-1, -1), 6);

	int x = 0;
	int y = 0;

	if (foundShape == false) {
		int blackPixels = 0, whitePixels = 0, averageX = 0, averageY = 0;
		for (int xShape = 0; xShape < bBox.width; xShape++){
			for (int yShape = 0; yShape < bBox.height; yShape++){
				if (imageCopy.at<uchar>(yShape + bBox.y, xShape + bBox.x) < 30) {
					blackPixels++;
				}
				else {
					whitePixels++;
					averageY += yShape + bBox.y;
					averageX += xShape + bBox.x;
				}
			}
		}
		double bwPixelRatio = 0.00;
		bwPixelRatio = double(whitePixels) / double(double(whitePixels) + double(blackPixels));
		
		

		//square
		if (bwPixelRatio > 0.7) {
			cout << "It a square!" << endl;
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
				shotShape = 5;
				foundShape = true;
			}
			//Regular shot
			else {
				shotShape = 1;
				foundShape = true;
			}
		}
		
		if (foundShape == false) {
			double curveLength = 0.00;
			curveLength = arcLength(contours.at(0), true);
			curveLength /= double(double(bBox.width) + double(bBox.height));
			cout << "Length: " << curveLength << endl;

			//Triangle shot aka Big bomb
			if (foundShape == false & 1.4 < curveLength & curveLength < 1.75) {
				cout << "it a triangle" << endl;
				shotShape = 4;
				foundShape = true;
				//x = 1;
				//y = 1;
			}

			//Plus shot aka cross shot
			else if (foundShape == false) {
				cout << "it a cross" << endl;
				shotShape = 3;
				foundShape = true;
				x = 1;
				y = 1;
			}
		}
	}

	//Part that finds the position of the shot
	vector<int> vlines;
	vector<int> hlines;
	for (int k = 0; k < 11; k++) {
		hlines.push_back((double(image.rows) / 10.0) * k);
	}
	for (int k = 0; k < 21; k++) {
		vlines.push_back((double(image.cols) / 20.0) * k);
	}


	//Find x and y
	//Find x
	for (int j = 0; j < vlines.size() - 1; j++) {
		if (vlines.at(j) < bBox.x & bBox.x <= vlines.at(j + 1)) {
			cout << "setting x to: " << j << ", from: " << x << endl;
			x += j;
		}
	}
	//Find y
	for (int j = 0; j < hlines.size() - 1; j++) {
		if (hlines.at(j) < bBox.y & bBox.y <= hlines.at(j + 1)) {
			cout << "setting y to: " << j << ", from: " << y << endl;
			y += j;
		}
	}
	if (x > 19) x = 19;
	if (y > 9) y = 9;
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
	approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true) * 0.02, true);
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

vector<int> scanForShot(VideoCapture cap, Gridfinder& gridfinder) {
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
	dilate(mask, mask, Mat(), Point(-1, -1), 2);
	dilate(mask, mask, Mat(), Point(-1, -1), 4);
	erode(mask, mask, Mat(), Point(-1, -1), 4);

	for (int x = 0; x < mask.cols; x++) {
		mask.at<uchar>(0, x) = 0;
		mask.at<uchar>(mask.rows-1, x) = 0;
	}
	for (int y = 0; y < mask.rows; y++) {
		mask.at<uchar>(y, 0) = 0;
		mask.at<uchar>(y, mask.cols-1) = 0;
	}

	//mask = makeGreenMask(mask);
	vector<vector<Point>> contours;
	imshow("Le mask: ", mask);
	contours = edgeDetection(mask, image);
	vector<vector<Point>> new_contours;
	for (int i = 0; i < contours.size(); i++) {
		
		vector<Point> approx;
		approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true)*0.02, true);
		//cout << "area: " << fabs(contourArea(contours.at(0))) << endl;
		if (!(fabs(contourArea(contours.at(i))) < 10 /*|| !isContourConvex(approx)*/)) {
			new_contours.push_back(contours.at(0));
			//cout << "Contour: " << new_contours.at(0) << endl;
		}
	}
	
	contours = new_contours;

	vector<int> shot;
	if (!(contours.empty())) {
		int edgeCount = shapeDetection(contours);
		shot = findShot(edgeCount, contours, mask, cap, gridfinder);
	}
	return shot;
}