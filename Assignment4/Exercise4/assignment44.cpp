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

struct TransformPoints
{
	std::vector<cv::Point2f> source;
	std::vector<cv::Point2f> target;
};

TransformPoints rectangles[2];

void thresh_callback(int, void*);

void onmouse(int event, int x, int y, int flags, void* userdata)
{
	int window = (int)userdata;

	if (event == CV_EVENT_MOUSEMOVE)
	{
		return;
	}

	std::cout << "Onmouse: (" << x << ", " << y << "): " << event << std::endl;
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		rectangles[window].source.push_back(Point2f(x, y));
		break;
	case CV_EVENT_LBUTTONUP:
		rectangles[window].source.push_back(Point2f(x, y));
		break;
	}

	if (rectangles[window].source.size() > 4)
	{
		rectangles[window].source.clear();
	}

	if (rectangles[window].source.size() > 1)
	{
		intermed[window] = src[window].clone();
		for (size_t i = 1; i < rectangles[window].source.size(); i += 2)
		{
			line(intermed[window], rectangles[window].source[i - 1], rectangles[window].source[i], Scalar(255, 0, 0));
		}
		imshow("src[" + std::to_string(window) + "]", intermed[window]);
	}

	thresh_callback(0, nullptr);
}

int main(int argc, char** argv)
{
	if (argc < 2) 
	{
		src[0] = imread("wall1.jpg");
		src[1] = imread("wall2.jpg");
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

	namedWindow("src[0]", CV_WINDOW_NORMAL);
	imshow("src[0]", intermed[0]);
	namedWindow("src[1]", CV_WINDOW_NORMAL);
	imshow("src[1]", intermed[1]);

	setMouseCallback("src[0]", onmouse, (void*)0);
	setMouseCallback("src[1]", onmouse, (void*)1);

	//thresh_callback(0, 0);

	waitKey(0);
	return(0);
}

#define PRINT_EX(a) {try{a;}catch(const Exception& ex){std::cout<<ex.what()<<std::endl;}}

void thresh_callback(int, void*)
{
	for (size_t i = 0; i < 2; ++i)
	{
		if (rectangles[i].source.size() != 4)
		{
			continue;
		}

		rectangles[i].target.clear();
		for (size_t j = 0; j < 4; ++j)
		{
			Point2f point = rectangles[i].source[j];
			if (j % 2 == 1)
			{
				point.y = rectangles[i].source[j - 1].y;
			}

			rectangles[i].target.push_back(point);
		}

		Matx33f transform = getPerspectiveTransform(rectangles[i].source.data(), rectangles[i].target.data());

		std::cout << "transform" << i << ":" << std::endl;
		std::cout << transform << std::endl;

		// fit dst to warped image
		{
			// calculate warped position of all corners
			cv::Point3f a = transform.inv() * (cv::Point3f(0, 0, 1));
			a = a * (1.0 / a.z);

			cv::Point3f b = transform.inv() * cv::Point3f(0, src[i].rows, 1);
			b = b * (1.0 / b.z);

			cv::Point3f c = transform.inv() * cv::Point3f(src[i].cols, src[i].rows, 1);
			c = c * (1.0 / c.z);

			cv::Point3f d = transform.inv() * cv::Point3f(src[i].cols, 0, 1);
			d = d * (1.0 / d.z);

			// to make sure all corners are in the image, every position must be > (0, 0)
			float x = ceil(abs(min(min(a.x, b.x), min(c.x, d.x))));
			float y = ceil(abs(min(min(a.y, b.y), min(c.y, d.y))));

			// and also < (width, height)
			float width = ceil(abs(max(max(a.x, b.x), max(c.x, d.x)))) + x;
			float height = ceil(abs(max(max(a.y, b.y), max(c.y, d.y)))) + y;

			Size size(width, height);

			dst[i] = Mat::zeros(size, src[i].type());
		}

		warpPerspective(src[i], dst[i], transform, dst[i].size());

		namedWindow("dst" + std::to_string(i), CV_WINDOW_NORMAL);
		imshow("dst" + std::to_string(i), dst[i]);
	}
}
