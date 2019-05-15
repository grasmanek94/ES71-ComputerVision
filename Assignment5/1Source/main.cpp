#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;

//main functions
void processImage();
void displayGraphics();

const int imgWidth = 1200;
const int imgHight = 800;

Mat frame1;
Mat frame2;
Mat frame3;
Mat result;

std::vector<Point> points1 {Point(822,685), Point(1030,690), Point(1140,450), Point(995,66), Point(820,95)};
std::vector<Point> points2 {Point(199,692), Point(405,692),  Point(518,468),  Point(427,95), Point(257,90),
							Point(907,700), Point(1113,700), Point(1063,103), Point(889,101),Point(785,470)};
std::vector<Point> points3 {Point(221,672), Point(416,660),  Point(391,94),	  Point(223,69), Point(104,444)};

int nPointsSelected = 0;

static void onMouse (int event, int x, int y, int flags, void* param)
{
	if(event != EVENT_LBUTTONDBLCLK)
		return;
	
	std::vector<Point>* points = (std::vector<Point>*)param;
	nPointsSelected ++;
	points->push_back(Point(x,y));
	
	
	processImage();
	displayGraphics();
}

int main(int argc, char *argv[])
{
	//create a window
	namedWindow("Frame1");
	namedWindow("Frame2");
	namedWindow("Frame3");
	namedWindow("Result", WINDOW_NORMAL);

	//load the image
	frame1 = imread("pavilionLeft.jpg"); 
	frame2 = imread("pavilionCenter.jpg"); 
	frame3 = imread("pavilionRight.jpg"); 
	
	resize(frame1, frame1, Size(imgWidth,imgHight));
	resize(frame2, frame2, Size(imgWidth,imgHight));
	resize(frame3, frame3, Size(imgWidth,imgHight));
		
	if(frame1.empty() || frame2.empty() || frame3.empty())
		std::cout << "Error opening file" << std::endl;

	setMouseCallback("Frame1", onMouse, (void*)&points1);
	setMouseCallback("Frame2", onMouse, (void*)&points2);
	setMouseCallback("Frame3", onMouse, (void*)&points3);

	processImage();
	displayGraphics();

	waitKey(0);

	destroyAllWindows();
	
	return 0;
}

void displayGraphics()
{
	// Display the resulting frame
	imshow( "Frame1", frame1 );
	imshow( "Frame2", frame2 );
	imshow( "Frame3", frame3 );
	if(!result.empty())
		imshow( "Result", result );
}

void processImage()
{
	std::cout << "number of points selected: " << nPointsSelected << std::endl;
		
	if( points1.size() < 4 ||
		points2.size() < 8 ||
		points3.size() < 4 )
		return;
	
	std::vector<Point> center1;
	std::vector<Point> center3; 
	
	if(points1.size()+points3.size() == points2.size())
	{
		for (int i = 0; i < points1.size(); i++)
		{
			center1.push_back(points2[i]);
		}
		for (int i = 0; i < points3.size(); i++)
		{
			center3.push_back(points2[i+points1.size()]);
		}
	}
	else
	{
		std::cout << "mismatch in points selected" << std::endl;
		std::cout << "points 1: " << points1.size()<< std::endl;
		std::cout << "points 2: " << points2.size()<< std::endl;
		std::cout << "points 3: " << points3.size()<< std::endl;
		return;
	}
	
	Point tar = Point(imgWidth,0);
	std::vector<Point> points6 {Point(0,0),Point(0,100),Point(100,100),Point(100,0)};
	std::vector<Point> center6 = points6;

	for(auto& i: center1) i += tar;
	for(auto& i: center6) i += tar;
	for(auto& i: center3) i += tar;
	
	Mat leftMatch  = findHomography(points1, center1, 0);
	Mat centMatch  = findHomography(points6, center6, 0);
	Mat rightMatch = findHomography(points3, center3, 0);
	
	Size size = Size(imgWidth*3,imgHight);
	result = Mat::zeros(size, frame2.type());

	warpPerspective(frame2, result, centMatch,  size, INTER_LINEAR, BORDER_TRANSPARENT);
	warpPerspective(frame1, result, leftMatch,  size, INTER_LINEAR, BORDER_TRANSPARENT);
	warpPerspective(frame3, result, rightMatch, size, INTER_LINEAR, BORDER_TRANSPARENT);
	
		
}
