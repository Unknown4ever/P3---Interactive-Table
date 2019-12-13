#pragma once
#include <opencv2\opencv.hpp>


using namespace cv;
using namespace std;


Mat makeMask(Mat image, int lowLim1[], int upLim1[]) {
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

Mat makeGreenMask(Mat image) {
	Mat mask;

	if (image.empty()) {
		//cout << "Could no find image" << endl;
	}
	else {
		image.copyTo(mask);
		for (int y = 0; y < mask.rows; y++) {
			for (int x = 0; x < mask.cols; x++) {
				int b = mask.at<Vec3b>(y, x)[0];
				int g = mask.at<Vec3b>(y, x)[1];
				int r = mask.at<Vec3b>(y, x)[2];

				if (b < g & r < g & g > 60) {
					mask.at<Vec3b>(y, x) = { 255, 255, 255 };
				}
				else {
					mask.at<Vec3b>(y, x) = 0;
				}
			}
		}
		cvtColor(mask, mask, COLOR_BGR2GRAY);
		erode(mask, mask, Mat(), Point(-1, -1), 3);
		dilate(mask, mask, Mat(), Point(-1, -1), 4);

	}
	return mask;
}

Mat contrastBrightness(Mat image, double contrast, int brightness) {
	Mat new_image = Mat::zeros(image.size(), image.type());
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			for (int c = 0; c < image.channels(); c++) {
				new_image.at<uchar>(y, x) =
					saturate_cast<uchar>(contrast * image.at<uchar>(y, x) + brightness);
			}
		}
	}
	return new_image;
}

Mat makeGrayMask(Mat image) {
	Mat grayim;
	cvtColor(image, grayim, COLOR_BGR2GRAY);

	grayim = contrastBrightness(grayim, 2, 1);

	cvtColor(grayim, grayim, COLOR_GRAY2BGR);

	int upper = 220;
	int skibidi = 4;

	inRange(grayim, Scalar(0, 0, 0), Scalar(upper, upper, upper), grayim);

	erode(grayim, grayim, Mat(), Point(-1, -1), 1);
	dilate(grayim, grayim, Mat(), Point(-1, -1), skibidi);
	erode(grayim, grayim, Mat(), Point(-1, -1), skibidi);
	dilate(grayim, grayim, Mat(), Point(-1, -1), 6);
	erode(grayim, grayim, Mat(), Point(-1, -1), 6);

	for (int x = 0; x < grayim.cols; x++) {
		grayim.at<uchar>(0, x) = 0;
		grayim.at<uchar>(grayim.rows - 1, x) = 0;
	}
	for (int y = 0; y < grayim.rows; y++) {
		grayim.at<uchar>(y, 0) = 0;
		grayim.at<uchar>(y, grayim.cols - 1) = 0;
	}

	return grayim;
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
	Mat lineFoto;
	image.copyTo(lineFoto);
	for (int i = 0; i < contours.size(); i += 2) {
		vector<Point> approx;
		approxPolyDP(Mat(contours.at(0)), approx, arcLength(Mat(contours.at(0)), true)*0.02, true);
		if (!(fabs(contourArea(contours.at(i))) < 80 /* || !isContourConvex(approx) */)) {
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
			for (int k = 0; k < 11; k++) {
				hlines.push_back((double(image.rows) / 10.0)*k);
			}
			for (int k = 0; k < 21; k++) {
				vlines.push_back((double(image.cols) / 20.0)*k);
			}

			

			int x = 0;
			int y = 0;
			//Find x and y
			//Vertical
			if (direction == true) {
				//Find x
				for (int j = 0; j < vlines.size() - 1; j++) {
					if (vlines.at(j) < bBox.x & bBox.x < vlines.at(j + 1)) {
						x = j;
					}
				}
				//Find y
				for (int j = 0; j < hlines.size() - 1; j++) {
					if (hlines.at(j) < bBox.y & bBox.y < hlines.at(j + 1)) {
						y = j;
					}
				}
			}
			//Horisontal
			else {
				//Find x
				for (int j = 0; j < vlines.size() - 1; j++) {
					if (vlines.at(j) < bBox.x & bBox.x < vlines.at(j + 1)) {
						x = j;
					}
				}
				//Find y
				for (int j = 0; j < hlines.size() - 1; j++) {
					if (hlines.at(j) < bBox.y & bBox.y < hlines.at(j + 1)) {
						y = j;
					}
				}
			}

			

			for (int k = 0; k < hlines.size(); k++) {
				line(lineFoto, Point(0, hlines.at(k)), Point(lineFoto.cols - 1, hlines.at(k)), Scalar(255, 0, 0));
			}
			for (int k = 0; k < vlines.size(); k++) {
				line(lineFoto, Point(vlines.at(k), 0), Point(vlines.at(k), lineFoto.rows - 1), Scalar(0, 255, 0));
			}

			circle(lineFoto, Point(bBox.x, bBox.y), 3, Scalar(0, 0, 255));

			imshow("ShipDetect 177", lineFoto);

			vector<int> boat;
			boat.push_back(x);
			boat.push_back(y);
			boat.push_back(boatSize);
			boat.push_back(direction);

			boats.push_back(boat);
		}
	}
	return boats;
}


vector<vector<Point>> edgeDetection(Mat mask, Mat image) {
	Mat canny_output;
	image.copyTo(canny_output);
	int thresh = 80;
	int max_thresh = 255;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Canny(mask, canny_output, thresh, thresh * 2, 3);
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	
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


vector<vector<int>> scanForBoats(Mat image) {
	Mat mask;
	image.copyTo(mask);
	int upLim[] = { 80,220,240 };
	int lowLim[] = { 30,50,0 };
	//mask = makeGreenMask(mask);
	mask = makeGrayMask(mask);

	imshow("Ship detect: mask", mask);
	vector<vector<int>> boats;
	vector<vector<Point>> contours;
	contours = edgeDetection(mask, image);
	boats = findBoats(contours, image);
	return boats;
}