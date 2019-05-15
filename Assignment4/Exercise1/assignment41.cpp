#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src[2];
Mat src_gray[2];
Mat dst[4];
std::vector<cv::Point2f> rectangles[2];
int thresh = 14;
int blur_val = 3;
int max_blur = 10;
int max_thresh = 255;
RNG rng(12345);

void thresh_callback(int, void*);

int main(int argc, char** argv)
{
	if (argc < 2) 
	{
		src[0] = imread("ImageA.jpg");
		src[1] = imread("ImageB.jpg");
	} else {
		src[0] = imread(argv[1]);
		src[1] = imread(argv[2]);
	}

	if (!src[0].data || !src[1].data)
	{
		std::cout << "Error reading images" << std::endl;
		return 1;
	}

	/*namedWindow("src[0]", CV_WINDOW_AUTOSIZE);
	imshow("src[0]", src[0]);
	namedWindow("src[1]", CV_WINDOW_AUTOSIZE);
	imshow("src[1]", src[1]);

	createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
	createTrackbar(" Blur:", "Source", &blur_val, max_blur, thresh_callback);*/

	thresh_callback(0, 0);

	waitKey(0);
	return(0);
}

#define PRINT_EX(a) {try{a;}catch(const Exception& ex){std::cout<<ex.what()<<std::endl;}}

void thresh_callback(int, void*)
{
	for (size_t i = 0; i < 2; ++i)
	{
		Mat bgdModel;
		Mat fgdModel;
		Mat mask;
		Mat result;
		dst[i] = Mat::zeros(src[i].size(), src[i].type());

		Rect rect;
		rect.x = ((double)src[0].size().width) * 0.075;
		rect.y = ((double)src[0].size().height) * 0.050;
		rect.width = ((double)src[0].size().width) * 0.50;
		rect.height = ((double)src[0].size().height) * 0.900;

		Mat white(Mat::zeros(src[i].size(), CV_8UC3));
		white.setTo(CV_RGB(255, 255, 255));

		PRINT_EX(grabCut(src[i], mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT));

		Mat mask2 = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

		Mat grabcut;
		src[i].copyTo(grabcut, mask2);
		namedWindow("grabcut" + std::to_string(i), CV_WINDOW_AUTOSIZE);
		imshow("grabcut" + std::to_string(i), grabcut);

		white.copyTo(result, mask2);

		namedWindow("binary" + std::to_string(i), CV_WINDOW_AUTOSIZE);
		imshow("binary" + std::to_string(i), result);

		cvtColor(result, src_gray[i], COLOR_BGR2GRAY);

		if (blur_val > 0)
		{
			GaussianBlur(src_gray[i], src_gray[i], Size((blur_val * 2) + 1, (blur_val * 2) + 1), 0.0);
		}

		Mat canny;

		Canny(src_gray[i], canny, thresh, thresh * 10, 3);

		namedWindow("canny" + std::to_string(i), CV_WINDOW_AUTOSIZE);
		imshow("canny" + std::to_string(i), canny);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(canny.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		std::vector<cv::Point> approx;
		size_t limit = contours.size();
		for (int j = 0; j < limit; j++)
		{
			cv::approxPolyDP(cv::Mat(contours[j]), approx, cv::arcLength(cv::Mat(contours[j]), true)*0.02, true);
			if (approx.size() == 4)
			{
				for(int k = 0; k < approx.size(); ++k)
				{
					rectangles[i].push_back(approx[k]);
				}
				contours.push_back(approx);
				drawContours(dst[i], contours, contours.size() - 1, Scalar(0,255,0));
			}
		}

		namedWindow("dst" + std::to_string(i), CV_WINDOW_AUTOSIZE);
		imshow("dst" + std::to_string(i), dst[i]);
	}

	Mat transform = getAffineTransform(rectangles[0].data(), rectangles[1].data());
	std::cout << transform << std::endl;

	dst[2] = Mat::zeros(src[0].size(), src[0].type());
	dst[3] = Mat::zeros(src[0].size(), src[0].type());
	warpAffine(dst[0], dst[2], transform, dst[2].size());
	warpAffine(src[0], dst[3], transform, dst[3].size());
	namedWindow("dst2", CV_WINDOW_AUTOSIZE);
	imshow("dst2", dst[2]);
	namedWindow("dst3", CV_WINDOW_AUTOSIZE);
	imshow("dst3", dst[3]);
}
