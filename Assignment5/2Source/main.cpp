#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"
#include <iostream>
#include <math.h>

using namespace cv;
using namespace xfeatures2d;

//main functions
void processImage();
void displayGraphics();

const int imgWidth = 1200;
const int imgHight = 800;
const int nPics = 3;

Mat frame[nPics];
Mat result;


int main(int argc, char *argv[])
{
	//create a window
	namedWindow("Result");

	//load the image
	frame[0] = imread("pavilionLeft.jpg"); 
	frame[1] = imread("pavilionCenter.jpg"); 
	frame[2] = imread("pavilionRight.jpg"); 
		
	if(frame[0].empty()||frame[1].empty()||frame[2].empty())
	{
		std::cout << "Error opening file" << std::endl;
		return -1;
	}
	
	for (int i = 0; i < nPics; i++)
		resize(frame[i], frame[i], Size(imgWidth,imgHight));

	processImage();
	displayGraphics();

	waitKey(0);

	destroyAllWindows();
	
	return 0;
}

void displayGraphics()
{
	imshow( "Result", result );
}

void processImage()
{
	std::vector<KeyPoint> keyPoints[nPics];
	Mat discriptors[nPics];
	
	for (int i = 0; i < nPics; i++)
	{
		int minHessian = 400;
		Ptr<SURF> surfer = SURF::create(minHessian);
		surfer->detect(frame[i], keyPoints[i]);
		
		surfer->compute(frame[i], keyPoints[i], discriptors[i]);
	}	
	
	std::vector<Point> points[nPics];
	
	Mat transformMatrix[nPics];
	
	for (int i = 0; i < nPics; i++)
	{
		BFMatcher matcher;
		std::vector< DMatch > matches;
		matcher.match(discriptors[i], discriptors[1], matches);
		
		double minDist = 100;
		
		for (int j = 0; j < discriptors[i].rows ; j++)
		{	
			double dist = matches[j].distance;
			if( dist < minDist ) 
				minDist = dist;
		}
		
		std::vector< DMatch > goodMatches;
		
		for (int j = 0; j < discriptors[i].rows ; j++)
		{
			if(matches[j].distance < 3*minDist+0.000001)
				goodMatches.push_back(matches[j]);
		}
		std::cout<<"min distance: "<< minDist << std::endl;
		std::vector<Point> pointsRef;
		
		for (int j = 0; j < goodMatches.size(); j++)
		{
			points[i].push_back(keyPoints[i][goodMatches[j].queryIdx].pt);
			pointsRef.push_back(keyPoints[1][goodMatches[j].trainIdx].pt);
		}
		
		std::cout<<"n Points [i]: "<<points[i].size()<<", n points ref: "<<pointsRef.size()<<std::endl;
		
		Point tar = Point(imgWidth,0);
		for(auto& j: pointsRef) j += tar;
		transformMatrix[i] = findHomography(points[i], pointsRef,CV_RANSAC);
	}
	
	//out of for loop to pase 2 in the middle	
	Size size = Size(imgWidth*3,imgHight);						
	warpPerspective(frame[1], result, transformMatrix[1], size, INTER_LINEAR, BORDER_TRANSPARENT);
	warpPerspective(frame[0], result, transformMatrix[0], size, INTER_LINEAR, BORDER_TRANSPARENT);
	warpPerspective(frame[2], result, transformMatrix[2], size, INTER_LINEAR, BORDER_TRANSPARENT);

	
		resize(result, result, Size(imgWidth*2,imgHight*2/3));
}
