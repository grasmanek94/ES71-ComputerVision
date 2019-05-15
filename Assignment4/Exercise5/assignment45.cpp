#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <vector>

using namespace cv;

Mat src[2];
Mat intermed[2];
Mat dst[2];

std::vector<cv::Point2f> rectangles = {
	Point2f(87, 139),
	Point2f(530, 125),
	Point2f(546, 259),
	Point2f(82, 270)
};

void thresh_callback(int, void*);

void onmouse(int event, int x, int y, int flags, void* userdata)
{
	if (event == CV_EVENT_MOUSEMOVE)
	{
		return;
	}

	if (rectangles.size() == 4)
	{
		rectangles.clear();
	}

	std::cout << "Onmouse: (" << x << ", " << y << "): " << event << std::endl;
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		rectangles.push_back(Point2f(x, y));
		break;
	case CV_EVENT_LBUTTONUP:
		rectangles.push_back(Point2f(x, y));
		break;
	}

	thresh_callback(0, nullptr);
}

int main(int argc, char** argv)
{
	if (argc < 2) 
	{
		src[0] = imread("Advertise1.jpg");
		src[1] = imread("Guus1.jpg");
	} else {
		src[0] = imread(argv[1]);
		src[1] = imread(argv[2]);
	}

	if (!src[0].data || !src[1].data)
	{
		std::cout << "Error reading images" << std::endl;
		return 1;
	}

	intermed[0] = src[0].clone();
	intermed[1] = src[1].clone();

	namedWindow("src[0]", CV_WINDOW_AUTOSIZE);
	imshow("src[0]", intermed[0]);
	namedWindow("src[1]", CV_WINDOW_AUTOSIZE);
	imshow("src[1]", intermed[1]);

	setMouseCallback("src[0]", onmouse);

	thresh_callback(0, 0);

	waitKey(0);
	return(0);
}

#define PRINT_EX(a) {try{a;}catch(const Exception& ex){std::cout<<ex.what()<<std::endl;}}

void thresh_callback(int, void*)
{
	if (rectangles.size() > 1)
	{
		intermed[0] = src[0].clone();
		for (size_t i = 1; i < rectangles.size(); i += 2)
		{
			line(intermed[0], rectangles[i - 1], rectangles[i], Scalar(255, 0, 0), 2);
		}
		imshow("src[0]", intermed[0]);
	}

	if (rectangles.size() != 4)
	{
		return;
	}

	std::vector<cv::Point2f> transformfix = rectangles;
	transformfix[1].y = transformfix[0].y;
	transformfix[3].y = transformfix[2].y;
	transformfix[3].x = transformfix[0].x;
	transformfix[2].x = transformfix[1].x;

	Matx33f transform = getPerspectiveTransform(rectangles.data(), transformfix.data());

	std::cout << "transform:" << std::endl;
	std::cout << transform << std::endl;

	int image = 0;
	// fit dst to warped image
	{
		// calculate warped position of all corners
		cv::Point3f a = transform.inv() * (cv::Point3f(0, 0, 1));
		a = a * (1.0 / a.z);

		cv::Point3f b = transform.inv() * cv::Point3f(0, src[image].rows, 1);
		b = b * (1.0 / b.z);

		cv::Point3f c = transform.inv() * cv::Point3f(src[image].cols, src[image].rows, 1);
		c = c * (1.0 / c.z);

		cv::Point3f d = transform.inv() * cv::Point3f(src[image].cols, 0, 1);
		d = d * (1.0 / d.z);

		// to make sure all corners are in the image, every position must be > (0, 0)
		float x = ceil(abs(min(min(a.x, b.x), min(c.x, d.x))));
		float y = ceil(abs(min(min(a.y, b.y), min(c.y, d.y))));

		// and also < (width, height)
		float width = ceil(abs(max(max(a.x, b.x), max(c.x, d.x)))) + x;
		float height = ceil(abs(max(max(a.y, b.y), max(c.y, d.y)))) + y;

		Size size(width, height);

		dst[image] = Mat::zeros(size, src[image].type());
	}

	warpPerspective(src[image], dst[image], transform, dst[image].size());

	dst[1] = Mat::zeros(src[1].size(), src[1].type());
	resize(src[1], dst[1], Size(abs(transformfix[1].x - transformfix[0].x), abs(transformfix[3].y - transformfix[0].y)));

	namedWindow("dst[1]", CV_WINDOW_AUTOSIZE);
	imshow("dst[1]", dst[1]);

	dst[1].copyTo(dst[0](cv::Rect(transformfix[0].x, transformfix[0].y, dst[1].cols, dst[1].rows)));

	namedWindow("dst[0]", CV_WINDOW_AUTOSIZE);
	imshow("dst[0]", dst[0]);

	Mat tmp(Mat::zeros(src[0].size(), src[0].type()));
	warpPerspective(dst[image], tmp, transform.inv(), tmp.size());

	namedWindow("tmp", CV_WINDOW_AUTOSIZE);
	imshow("tmp", tmp);
	/*namedWindow("dst[1]a", CV_WINDOW_AUTOSIZE);
	imshow("dst[1]a", dst[1]);

	Point2f copy_start = rectangles[0];
	cv::Point3f homogeneous = transform.inv() * copy_start;
	cv::Point2f result(homogeneous.x, homogeneous.y);

	dst[0] = src[0].clone();
	Mat tmp = dst[1].clone();
	resize(tmp, dst[1], Size(abs(rectangles[1].x - rectangles[0].x), abs(rectangles[3].y - rectangles[0].y)));

	namedWindow("dst[1]", CV_WINDOW_AUTOSIZE);
	imshow("dst[1]", dst[1]);

	dst[1].copyTo(dst[0](cv::Rect(result.x, result.y, dst[1].cols, dst[1].rows)));

	namedWindow("dst[0]", CV_WINDOW_AUTOSIZE);
	imshow("dst[0]", dst[0]);*/
}
