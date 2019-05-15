#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;

//main functions
void playVideo();
void processImage();
void displayGraphics();
int distancePoints(Point one, Point two);

Mat frame;
Mat result;

int main(int argc, char *argv[])
{
	std::cout << "Hello, OpenCV version "<< CV_VERSION << std::endl;
	
//create a window
namedWindow("Frame");
namedWindow("Result");

//load the image
if(argc > 1)
	frame = imread(argv[1]);	
else
	frame = imread("Hands.jpg"); 
	
if(frame.empty())
    std::cout << "Error opening video stream or file" << std::endl;

processImage();
displayGraphics();

waitKey(0);

destroyAllWindows();
  
return 0;
}

void displayGraphics()
{
	// Display the resulting frame
	imshow( "Frame", frame );
	imshow( "Result", result );
}

void processImage()
{
	Mat prosessing;
	Mat threshold_output;
	Mat HSV;
	
	////not working part
	//GaussianBlur( frame, prosessing, Size(11, 11),2, 2 );
	//threshold(prosessing, threshold_output,180,255,THRESH_BINARY_INV);
	
	// working part
	cvtColor(frame, HSV, COLOR_BGR2HSV);
	GaussianBlur(HSV, HSV, Size(11, 11), 2, 2);
	inRange(HSV, Scalar(0,0,0), Scalar(255,255,191), HSV);
    threshold( HSV, threshold_output, 100, 255, THRESH_BINARY );
	
	
	std::vector<std::vector<cv::Point>> contours;
	std::vector<Vec4i> hierarchy;
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE );
	std::cout<<contours.size()<<std::endl;
	result = frame.clone();
	;
	std::vector<std::vector<int>> hullI(contours.size());
	std::vector<std::vector<Vec4i>> defects(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		convexHull( contours[i], hullI[i], false);
		
		convexityDefects(contours[i], hullI[i], defects[i]);
		std::cout<<defects[i].size()<<std::endl;
	}
	
	std::string fingerName[5] = {"Pinky #5",
								 "Ring #4", 
								 "middle #3",
								 "index #2",
								 "thumb #1"};
	std::vector<Moments> mu(contours.size() );
	
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(255,255,255 );
		drawContours( result, contours, i, color, CV_FILLED, 8, hierarchy );
		
		Point centerOfMass;
		float mayorAxis;
		
		mu[i] = moments( contours[i], false );
		
		centerOfMass = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); 
		circle(result,centerOfMass,5,Scalar(255,0,150),5);
		mayorAxis = 180+(0.5*atan2((2 * mu[i].mu11), (mu[i].mu20 - mu[i].mu02))) * (180 / M_PI);
		
		
		std::vector<int> defPoint;
		
		for (int j = 0; j < defects[i].size(); j++)
		{
			Point start = contours[i][defects[i][j][0]];
			Point end   = contours[i][defects[i][j][1]];
			Point depth = contours[i][defects[i][j][2]];
			float depthI= defects[i][j][3];
			
			if(depthI>1000)
			{
				defPoint.push_back(j);
				if(depthI>50000)
				{
					circle(result,depth,4,Scalar(0,0,150),2);//depthPoint
				}/*
				circle(result,start,2,Scalar(150,0,0),2);//start
				circle(result,end,2,Scalar(0,150,0),2);//end
				std::cout<<"depth"<<defects[i][j][3]<<std::endl;
				line(result, start, depth, Scalar(150,150,0), 1, 8);
				line(result, end, depth, Scalar(0,150,150), 1, 8);
				line(result, start, end, Scalar(150,0,150), 1, 8);*/
				
			}
		}
		int finger = 0;
		for (int j = 0; j < defPoint.size(); j++)
		{
			int thisDef = defPoint[j];
			int prevDef;
			
			if(j==0)
				prevDef = defPoint[defPoint.size()-1];
			else
				prevDef = defPoint[j-1];
				
			Point tip1 = contours[i][defects[i][thisDef][0]];
			Point tip2 = contours[i][defects[i][prevDef][1]];
			Point tip  = (tip1+tip2)/2;
			
			Point base1 = contours[i][defects[i][thisDef][2]];
			Point base2 = contours[i][defects[i][prevDef][2]];
			Point base  = (base1+base2)/2;
			
			if(distancePoints(tip1, tip2) < 200)
			{			
				line(result, tip, base, Scalar(255,0,150), 3, 8);
				line(result, base, centerOfMass, Scalar(255,0,150), 3, 8);
				double angle = atan2(tip.y - base.y, tip.x - base.x)*180/CV_PI;
				
				if(angle<0)
					angle = angle*(-1);
				else
					angle = 360-angle;
				
				Point textPos = tip;
				putText(result, fingerName[finger], textPos, FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 2);
				textPos.y -= 30;
				putText(result, std::to_string(angle), textPos, FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 2);
				finger ++;
			}
			else
			{					
				line(result, tip, centerOfMass, Scalar(255,150,0), 3, 8);
				Point textPos = tip;
				putText(result, std::to_string(mayorAxis), textPos, FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 2);
			}
		}
	}
}

int distancePoints(Point one, Point two)
{
	int x = one.x-two.x;
	int y = one.y-two.y;
	int difference = sqrt(pow(x,2)+pow(y,2));
	return difference;
}
