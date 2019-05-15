#include <opencv2/opencv.hpp>

#define CATCH(a) try{ a } catch(const std::exception& ex){ std::cout << ex.what() << std::endl; }
#define degreesToRadians(angleDegrees) ((angleDegrees) * CV_PI / 180.0)
#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / CV_PI)

using namespace cv;

struct parameters
{
	int low_threshold;
	int high_threshold;
	int max_threshold;
	int gap;
	int max_gap;
	int line_threshold;
	int max_line_threshold;
	int erosion_method;
	int max_erosion_method;
	int erosion_size;
	int max_erosion_size;
	int gaus_size;
	int max_gaus_size;
	int dilate_method;
	int max_dilate_method;
	int dilate_size;
	int max_dilate_size;

	int circle_mindist;
	int circle_max_mindist;
	int circle_param1;
	int circle_max_param1;
	int circle_param2;
	int circle_max_param2;
	int circle_min_radius;
	int circle_max_radius;
	int circle_max_value;

	parameters()
	{
		low_threshold		= 19;
		high_threshold		= 255;
		max_threshold		= 255;
		gap					= 255;
		max_gap				= 255;
		line_threshold		= 180;
		max_line_threshold	= 255;
		erosion_method		= 0;
		max_erosion_method	= 2;
		erosion_size		= 0;
		max_erosion_size	= 255;
		gaus_size			= 12;
		max_gaus_size		= 12;
		dilate_method		= 0;
		max_dilate_method	= 2;
		dilate_size			= 0;
		max_dilate_size		= 255;
		circle_mindist		= 50;
		circle_max_mindist	= 255;
		circle_param1		= 200;
		circle_max_param1	= 1024;
		circle_param2		= 100;
		circle_max_param2	= 1024;
		circle_min_radius	= 0;
		circle_max_radius	= 1024;
		circle_max_value	= 1024;
	}
};

struct windows_names
{
	std::string params;
	std::string clean;
	std::string edges;
	std::string hough;
	std::string circlehough;

	windows_names()
	{
		params = "Parameters";
		clean = "Cleanup";
		edges = "Edges";
		hough = "Hough Lines";
		circlehough = "Circles";
	}
};

struct workspace
{
	Mat src;
	Mat src_gray;
	Mat dst_clean;
	Mat dst_edges;
	Mat dst_hough;
	Mat dst_circlehough;
};

workspace ws;
windows_names windows;
parameters params;

// lines
Scalar GetColor(int i, size_t max)
{
	if (max == 6)
	{
		return Scalar(
			255 * (i == 4 || i == 0 || i == 3), 
			255 * (i == 1 || i == 3 || i == 5), 
			255 * (i == 2 || i == 4 || i == 5)
		);
	}
	return Scalar(64 * (i % 5), 64 * ((1 + i) % 5), 64 * ((2 + i) % 5));
}

double Degrees(const Vec4i &v1, const Vec4i &v2, Point& r)
{
	// get cross
	Point o1(v1[0], v1[1]);
	Point p1(v1[2], v1[3]);
	Point o2(v2[0], v2[1]);
	Point p2(v2[2], v2[3]);

	Point x = o2 - o1;
	Point d1 = p1 - o1;
	Point d2 = p2 - o2;

	double cross_value = d1.x*d2.y - d1.y*d2.x;
	if (abs(cross_value) < /*EPS*/1e-8)
	{
		return 0.0;
	}

	double t1 = (x.x * d2.y - x.y * d2.x) / cross_value;
	Point b = 
		o1 + d1 * t1;
	Point a =
		cv::norm(b - o1) > cv::norm(b - p1) ? o1 : p1;
	Point c =
		cv::norm(b - o2) > cv::norm(b - p2) ? o2 : p2;

	// get angle
	Point ab = { b.x - a.x, b.y - a.y };
	Point cb = { b.x - c.x, b.y - c.y };

	double dot = (ab.x * cb.x + ab.y * cb.y); // dot product
	double cross = (ab.x * cb.y - ab.y * cb.x); // cross product

	double alpha = atan2(cross, dot);

	r = b;

	return floor(alpha * 180.0 / CV_PI + 0.5);
}

Point Mid(const Point &v1, const Point &v2, double distance = 0.5)
{
	return (v1 + v2) * distance;
}

Point Mid(const Vec4i &v1, const Vec4i &v2, double distance = 0.5)
{
	return Mid(
		Mid(Point(v1[0], v1[1]), Point(v1[2], v1[3])),
		Mid(Point(v2[0], v2[1]), Point(v2[2], v2[3])),
		distance
	);
}

std::pair<int, int> FindNeightbour(const std::vector<Vec4i>& lines, int location, int point, double max_distance = 25.0)
{
	int real_point = 2 * point;
	Point a(lines[location][real_point + 0], lines[location][real_point + 1]);

	for (int i = 0; i < lines.size(); ++i)
	{
		if (i == location)
		{
			continue;
		}

		for (int j = 0; j < 2; ++j)
		{
			int j_point = 2 * j;
			Point b(lines[i][j_point + 0], lines[i][j_point + 1]);
			if (cv::norm(b - a) < max_distance)
			{
				return std::pair<int, int>(i, j);
			}
		}
	}

	return std::pair<int, int>(location, point);
}

void HoughUpdate(int a, void* b)
{
	std::vector<Vec4i> lines;
	char counter[2] = { 'a', 0 };

	HoughLinesP(ws.dst_edges, lines, 1, CV_PI / 180, params.line_threshold, 0, params.gap);
	cvtColor(ws.dst_edges, ws.dst_hough, CV_GRAY2BGR);
	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(ws.dst_hough, Point(l[0], l[1]), Point(l[2], l[3]), GetColor(i, lines.size()), 3, CV_AA);
		
		for(int j = 0; j < 2; ++j)
		{
			std::pair<int, int> neightbour = FindNeightbour(lines, i, j);
			if (neightbour.first > i)
			{
				Vec4i p = lines[neightbour.first];
				Point r;
				double degrees = Degrees(l, p, r);
				Point mid = Mid(l, p);

				std::string corner
					= std::string(counter)
					+ "(" + std::to_string(r.x) + ", " + std::to_string(r.y) + ")"
					+ " " + std::to_string((int)degrees) + "deg";

				putText(ws.dst_hough, corner, mid,
					FONT_HERSHEY_COMPLEX_SMALL, 0.8, GetColor(i, lines.size()), 1, CV_AA);
				putText(ws.dst_hough, corner, mid - Point(1, 1),
					FONT_HERSHEY_COMPLEX_SMALL, 0.8, GetColor(i - 1, lines.size()), 1, CV_AA);

				++counter[0];
			}
		}
	}

	imshow(windows.hough, ws.dst_hough);
}

// circles

void CircleUpdate(int a, void* b)
{
	std::vector<Vec3f> circles;

	GaussianBlur(ws.dst_edges, ws.dst_circlehough, Size(1 + params.gaus_size * 2, 1 + params.gaus_size * 2), 0.0);
	HoughCircles(ws.dst_edges, circles, CV_HOUGH_GRADIENT, 1, params.circle_mindist, params.circle_param1, params.circle_param2, params.circle_min_radius, params.circle_max_radius);
	
	ws.dst_circlehough = ws.src.clone();

	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		circle(ws.dst_circlehough, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		circle(ws.dst_circlehough, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}

	imshow(windows.circlehough, ws.dst_circlehough);
}

// main

void EdgesUpdate(int a, void* b)
{
	Canny(ws.dst_clean, ws.dst_edges, params.low_threshold, params.high_threshold, 3);

	imshow(windows.edges, ws.dst_edges);

	CircleUpdate(a, b);
	HoughUpdate(a, b);
}

void CleanUpdate(int a, void* b)
{
	if (params.gaus_size > 0)
	{
		GaussianBlur(ws.src_gray, ws.dst_clean, Size(1 + params.gaus_size * 2, 1 + params.gaus_size * 2), 0.0);
	}
	else
	{
		ws.dst_clean = ws.src_gray.clone();
	}

	if (params.erosion_size > 0)
	{
		Mat element = getStructuringElement(params.erosion_method,
			Size(2 * params.erosion_size + 1, 2 * params.erosion_size + 1),
			Point(params.erosion_size, params.erosion_size));

		erode(ws.dst_clean, ws.dst_clean, element);
	}

	if (params.dilate_size > 0)
	{
		Mat element = getStructuringElement(params.dilate_method,
			Size(2 * params.dilate_size + 1, 2 * params.dilate_size + 1),
			Point(params.dilate_size, params.dilate_size));

		dilate(ws.dst_clean, ws.dst_clean, element);
	}

	imshow(windows.clean, ws.dst_clean);

	EdgesUpdate(a, b);
}

void CreateWindows()
{
	namedWindow(windows.params, CV_WINDOW_NORMAL);
	namedWindow(windows.clean, CV_WINDOW_AUTOSIZE);
	namedWindow(windows.edges, CV_WINDOW_AUTOSIZE);
	namedWindow(windows.hough, CV_WINDOW_AUTOSIZE);
	namedWindow(windows.circlehough, CV_WINDOW_AUTOSIZE);

	createTrackbar("Low Thres:", windows.params, &params.low_threshold, params.max_threshold, CleanUpdate);
	createTrackbar("High Thres:", windows.params, &params.high_threshold, params.max_threshold, CleanUpdate);
	createTrackbar("Max Gap:", windows.params, &params.gap, params.max_gap, CleanUpdate);
	createTrackbar("Line Threshold:", windows.params, &params.line_threshold, params.max_line_threshold, CleanUpdate);
	createTrackbar("Erosion Method:", windows.params, &params.erosion_method, params.max_erosion_method, CleanUpdate);
	createTrackbar("Erosion Size:", windows.params, &params.erosion_size, params.max_erosion_size, CleanUpdate);
	createTrackbar("Dilate Method:", windows.params, &params.dilate_method, params.max_dilate_method, CleanUpdate);
	createTrackbar("Dilate Size:", windows.params, &params.dilate_size, params.max_dilate_size, CleanUpdate);
	createTrackbar("Gaus Size:", windows.params, &params.gaus_size, params.max_gaus_size, CleanUpdate);

	createTrackbar("Circle Dmin:", windows.params, &params.circle_mindist, params.circle_max_mindist, CleanUpdate);
	createTrackbar("Circle P1:", windows.params, &params.circle_param1, params.circle_max_param1, CleanUpdate);
	createTrackbar("Circle P2:", windows.params, &params.circle_param2, params.circle_max_param2, CleanUpdate);
	createTrackbar("Circle Rmin:", windows.params, &params.circle_min_radius, params.circle_max_value, CleanUpdate);
	createTrackbar("Circle Rmax:", windows.params, &params.circle_max_radius, params.circle_max_value, CleanUpdate);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		return 1;
	}
	// "HoughPicture.jpg"
	// "EuroCoins.jpg"
	// "EuroCoins2.jpg"

	/// Load an image
	ws.src = imread(argv[1]);
	if (argv[1][0] == 'E')
	{
		params.gaus_size = 2;
	}

	/// Convert the image to grayscale
	cvtColor(ws.src, ws.src_gray, CV_BGR2GRAY);

	CreateWindows();

	CleanUpdate(0, nullptr);

	/// Wait until user exit program by pressing a key
	waitKey(0);

	return 0;
}
