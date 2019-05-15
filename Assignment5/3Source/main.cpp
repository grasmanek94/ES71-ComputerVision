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
	frame[0] = imread("clutteredDesk.jpg"); 
	frame[1] = imread("Box.jpg"); 
	frame[2] = imread("elephant.jpg"); 
		
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
	result = frame[0].clone();
	
	std::vector<KeyPoint> keyPoints[nPics];
	Mat discriptors[nPics];
	
	for (int i = 0; i < nPics; i++)
	{
		int minHessian = 400;
		Ptr<SURF> surfer = SURF::create(minHessian);
		surfer->detect(frame[i], keyPoints[i]);
		
		surfer->compute(frame[i], keyPoints[i], discriptors[i]);
	}	
	
	for (int i = 1; i < nPics; i++)
	{
		BFMatcher matcher;
		std::vector< DMatch > matches;
		matcher.match(discriptors[i], discriptors[0], matches);

		double minDist = 100;
		
		for (int j = 0; j < discriptors[i].rows ; j++)
		{	
			double dist = matches[j].distance;
			if (dist < minDist)
			{
				minDist = dist;
			}
		}
		
		std::vector< DMatch > goodMatches;
		
		for (int j = 0; j < discriptors[i].rows ; j++)
		{
			if (matches[j].distance < minDist*2.0)
			{
				goodMatches.push_back(matches[j]);
			}
		}
		std::cout<<"min distance: "<< minDist << std::endl;
		
		std::vector<Point> points;
		std::vector<Point> pointsRef;
		
		for (int j = 0; j < goodMatches.size(); j++)
		{
			points.push_back(keyPoints[i][goodMatches[j].queryIdx].pt);
			pointsRef.push_back(keyPoints[0][goodMatches[j].trainIdx].pt);
			//circle(result, keyPoints[i][goodMatches[j].queryIdx].pt, keyPoints[i][goodMatches[j].queryIdx].size, Scalar(255 * (i % 2), 255 * ((i + 1) % 2), 0, 0));
		}
		
		std::cout << "n Points [i]: " << points.size() << ", n points ref: " << pointsRef.size() << std::endl;
		
		Matx33f transformMatrix;
		transformMatrix = findHomography(points, pointsRef, CV_RANSAC);
		
		
		std::vector<Point2f> objectCorners(4);
		objectCorners = {Point2f(0,0),
						Point2f(frame[i].cols,0),
						Point2f(frame[i].cols,frame[i].rows),
						Point2f(0,frame[i].rows)};
		std::vector<Point2f> resultCorners(4);
		
		std::cout <<transformMatrix<<std::endl;
		std::cout << objectCorners << std::endl;
		std::cout << resultCorners << std::endl;

		perspectiveTransform(objectCorners, resultCorners, transformMatrix);
		
		line(result, resultCorners[0], resultCorners[1], Scalar(200,200,0));
		line(result, resultCorners[1], resultCorners[2], Scalar(200,200,0));
		line(result, resultCorners[2], resultCorners[3], Scalar(200,200,0));
		line(result, resultCorners[3], resultCorners[0], Scalar(200,200,0));
		
	}
}
