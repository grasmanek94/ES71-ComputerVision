#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

//main functions
void playVideo();
void processImage();
void displayGraphics();
void on_trackbar( int, void* );

//video
VideoCapture cap;
Mat frame;
Mat result;

int main(int argc, char *argv[])
{
//create a window
namedWindow("Frame");
namedWindow("Result");

//load the image
if(argc > 1)
	cap = VideoCapture(argv[1]);	
else
	cap = VideoCapture("SingleColors.mp4"); 
	
if(!cap.isOpened())
    std::cout << "Error opening video stream or file" << std::endl;


playVideo();


waitKey(0);

cap.release();
destroyAllWindows();
  
return 0;
}

void playVideo()
{
	while(1){
 
		cap >> frame;
	  
		// If the frame is empty, break immediately
		if (frame.empty())
			break;
	 
		processImage();
		displayGraphics();

		// Press  ESC on keyboard to exit
		char c=(char)waitKey(25);
		if(c==27)
			break;
	}
}

void displayGraphics()
{
	// Display the resulting frame
	imshow( "Frame", frame );
	imshow( "Result", result );
}

void processImage()
{
	Mat prosessing = frame.clone();
	
	cvtColor(prosessing, result, COLOR_BGR2HSV);
	int nColorsDetect = 3;
	
	int range[nColorsDetect][6] = {	{170,100,100, 185,255,255},
									{ 18,150,100,  25,255,255},
									{100,130,050, 120,255,255}};
	
	int balcolor[nColorsDetect][3] = {  {0,0,255},
										{0,255,0},
										{255,0,0}};
	
	prosessing = result.clone();
	result = frame.clone();
	for (int i = 0; i < nColorsDetect; i++)
	{
		Mat color;
		inRange(prosessing,Scalar(range[i][0],range[i][1],range[i][2])
				,Scalar(range[i][3],range[i][4],range[i][5]),color);
		
		Mat blurred;
		GaussianBlur(color, blurred, Size(9,9), 2,2);
		std::vector<Vec3f> circles;
		HoughCircles(blurred, circles, CV_HOUGH_GRADIENT, 1,50, 210,12,10,50);
		
		for (int j = 0; j < circles.size(); j++)
		{
			 Point center(cvRound(circles[j][0]), cvRound(circles[j][1]));
			 int radius = cvRound(circles[j][2]);
			 std::string text = std::string("middle here: ") + std::to_string(center.x)+std::string(",") + std::to_string(center.y);
			 putText(result, text, Point(center.x-10 ,center.y-10), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 2);
			 //X marks the spot
			 line(result, Point(center.x-10 ,center.y+10), Point(center.x+10, center.y-10), Scalar(255,255,255), 3);
			 line(result, Point(center.x+10 ,center.y+10), Point(center.x-10, center.y-10), Scalar(255,255,255), 3);
		}
	}
}

