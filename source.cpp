#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include "Serial.h"
using namespace cv;
using namespace std;

const int hight = 150;


int thresh = 100;



vector<Vec2f> lines;

CSerial serial;
VideoCapture cap;
void jump(void);
uint8_t findLine(unsigned int * y);
bool not_empty(Mat src);
Mat frame, Gray_frame, BW_frame, Opened_frame, Closed_frame, edges, BW2, window, BW;

int windowXPosition = 200;
int counter = 0;

float alpha = 1, beta = 0;
int main(int argc, char** argv)
{
	if (serial.Open(5, 9600))                      //open serial port to comunicate with arduino
		cout << "Port opened successfully" << endl;
	else
		cout << "Failed to open port!" << endl;

	// open the default camera
	cap = VideoCapture(1);


	if (!cap.open(1))   //check that the camera is open
		return 0;


	// structuring elements for opening and closing operations
	cv::Size_<int> struct_size(3, 3);
	cv::Size_<int> struct_size2(5, 5);

	Mat element = getStructuringElement(MORPH_RECT, struct_size);
	Mat element2 = getStructuringElement(MORPH_RECT, struct_size2);


	Mat imageROI;

	unsigned int lineYPosition = 200;

	uchar* p;

	while (findLine(&lineYPosition));                   // the function returns 1 when it can't find lines 

	while (1)
	{
		cap >> frame;

		cvtColor(frame, Gray_frame, CV_RGB2GRAY);									   // convert frame to gray scale
		static Mat adjustedImage = Mat::zeros(Gray_frame.size(), Gray_frame.type());

		threshold(Gray_frame, BW, 100, 255.0, THRESH_BINARY);                        // convert to binary image
																					 // creating subimage to detect obstacles at certain point
		window = Gray_frame(Rect(100, lineYPosition + 70, 50, 50));
		
		imageROI = BW(Rect(0, lineYPosition - (hight + 3), BW.cols, hight));

		//apply opening and closing to remove noise and unwanted objects

		cv::morphologyEx(imageROI, Opened_frame, MORPH_OPEN, element);
		cv::morphologyEx(Opened_frame, Closed_frame, MORPH_CLOSE, element2);
		window = Closed_frame(Rect(windowXPosition, 100, 100, 50));

		int step = 3;
		if (not_empty(window)) {

			jump();

			counter++;
			if (counter > 7)
			{
				windowXPosition += step;                                                      // compensate for dino acceleration
				counter = 0;
			}
			if (windowXPosition > 280 && windowXPosition < 350) windowXPosition = 350;
			cout << "position : " << windowXPosition << endl;
			if (windowXPosition > 600) windowXPosition = 600;

		}
		imshow("binary", ~Closed_frame);
		imshow("window", ~window);

		if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
	}
	return 0;
}

// this function sends a character to arduino as a command to jump
void jump(void)
{
	serial.SendData("L", 1);

}
// this function takes a binary image and returns true if the image contains any white pixels
bool not_empty(Mat src) {
	uchar* p;
	for (int i = 0; i < src.rows; ++i)
	{
		p = src.ptr<uchar>(i);
		for (int j = 0; j < src.cols; ++j)
		{
			if (p[j] == 0)
				return true;
		}
	}
	return false;
}

//this function outputs the height of a horizontal line if found in the image. and returns 1 if no line was found.
uint8_t findLine(unsigned int * y)
{
	cap >> frame;																   // read the next frame from camera
	cvtColor(frame, Gray_frame, CV_RGB2GRAY);									   // convert frame to gray scale
	Canny(Gray_frame, edges, 50, 200, 3);										   // get edges needed for the houghline function to extract the line
	threshold(edges, BW_frame, 95.0, 255.0, THRESH_BINARY);
	HoughLines(BW_frame, lines, 1, CV_PI / 180, 200, 0, 0);
	if (lines.size() == 0) return 1;       // error : couldn't find any lines
	float rho = lines[0][0], theta = lines[0][1];
	Point pt1, pt2;
	double a = cos(theta), b = sin(theta);
	double x0 = a*rho, y0 = b*rho;
	*y = cvRound(y0 + 1000 * (a));
	return 0;
}


