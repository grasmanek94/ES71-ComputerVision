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
	cap = VideoCapture("MeasuringAngle.mp4"); 
	
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

	prosessing = result.clone();
	result = frame.clone();
	inRange(prosessing,Scalar(5,230,125)
			,Scalar(20,255,255),result);
			
	prosessing = result.clone();
	GaussianBlur(prosessing, result, Size(9,9), 2,2);
	
	prosessing = result.clone();
	Canny(prosessing, result, 100,100);
	
	prosessing = result.clone();
	std::vector<Vec2f> lines;
    HoughLines( result, lines, 1, CV_PI/180, 80 );
	
	int nLines = lines.size();
	float totalTheta = 0;
	for( size_t i = 0; i < nLines; i++ )
    {
		totalTheta +=lines[i][1];
	}
	
	result = frame.clone();
	float avgTheta = totalTheta*180/CV_PI/nLines;
	std::string text = std::string("object angle: ") + std::to_string((double)avgTheta);
	putText(result, text, Point(10,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 2);
}

