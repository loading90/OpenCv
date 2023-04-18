//#define _CRT_SECURE_NO_WARNINGS
//
//#include "opencv2/core.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//
//#include <iostream>
//
//using namespace cv;
//using namespace std;
//
//Mat src_gray;
//Mat src;
//
//int thresh = 100;
//int sqr = 7000;
//int destetation = 1;
//
//RNG rng(12345);
//
//void thresh_callback(int, void*);
//
//cv::VideoCapture* OpenCamera(int numCamera = 1)
//{
//	cv::VideoCapture* camera = new cv::VideoCapture(numCamera);
//	if (!camera->isOpened())
//	{
//		cout << "Error!" << endl;
//		system("pause");
//		exit(-1);
//	}
//
//	return camera;
//}
//
//int mainf(int argc, char** argv)
//{
//	VideoCapture* camera = OpenCamera();
//	cout << camera->get(CAP_PROP_FRAME_WIDTH) << " " << camera->get(CAP_PROP_FRAME_HEIGHT) << endl;
//
//	while (true)
//	{
//		*camera >> src;
//
//		cvtColor(src, src_gray, COLOR_BGR2GRAY);
//		blur(src_gray, src_gray, Size(3, 3));
//		const char* source_window = "Source";
//		namedWindow(source_window);
//		const int max_thresh = 255;
//		createTrackbar("Canny thresh:", source_window, &thresh, max_thresh, thresh_callback);
//		createTrackbar("Sqr:", source_window, &sqr, 10000, thresh_callback);
//		createTrackbar("Dest:", source_window, &destetation, 10, thresh_callback);
//		thresh_callback(0, 0);
//		imshow(source_window, src);
//		waitKey(2);
//	}
//
//	return 0;
//}
//
//bool Compare(RotatedRect first, RotatedRect second)
//{
//	Point2f tempFirst = first.size;
//	Point2f tempSecond = second.size;
//
//	int squareFirst = tempFirst.x * tempFirst.y;
//	int squareSecond = tempSecond.x * tempSecond.y;
//
//	if (squareFirst > squareSecond)
//		return true;
//	return false;
//}
//
//void thresh_callback(int, void*)
//{
//	Mat canny_output;
//	Canny(src_gray, canny_output, thresh, thresh * 2);
//	vector<vector<Point>> contours;
//	findContours(canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
//	vector<RotatedRect> minRect(contours.size());
//	vector<RotatedRect> minEllipse(contours.size());
//	for (size_t i = 0; i < contours.size(); i++)
//	{
//		minRect[i] = minAreaRect(contours[i]);
//		if (contours[i].size() > 5)
//		{
//			minEllipse[i] = fitEllipse(contours[i]);
//		}
//	}
//	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
//
//	std::sort(minRect.begin(), minRect.end(), Compare);
//	
//	Point2f pointMaxSquare = minRect[0].size;
//	int maxSquare = pointMaxSquare.x * pointMaxSquare.y;
//
//	for (size_t i = 0; i < minRect.size(); i++)
//	{
//		Scalar color = Scalar(0, 255, 255);//Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
//		Point2f rect_points[4];
//		minRect[i].points(rect_points);
//		Point2f square = minRect[i].size;
//		int temp = square.x * square.y;
//		double dest = square.x - square.y;
//		if (dest < 0.0)
//			dest *= -1;
//		if (temp > sqr && dest < destetation)
//		{
//			//cout << temp << endl;
//			for (int j = 0; j < 4; j++)
//			{
//				line(src, rect_points[j], rect_points[(j + 1) % 4], color, 2);
//			}
//		}
//	}
//}
//
//
///*#include <iostream>
//
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/imgproc.hpp"
//
//using namespace std;
//using namespace cv;
//
//int mainRect()
//{
//	std::cout << "enter camera number: " << std::endl;
//	int nCamera = 0; // номер камеры
//	std::cin >> nCamera;
//	cv::VideoCapture camera(nCamera);
//	if (!camera.isOpened()) {
//		// если не можем открыть камеру, выводим сообщение
//		printf("Cannot open initialize webcam or video file!\n");
//		return 0;
//	}
//
//	Mat dst, cdst, cdstP;
//	Mat src;
//
//	camera >> src;
//
//	while (true)
//	{
//		camera >> src;
//
//		// Edge detection
//		Canny(src, dst, 50, 200, 3);
//		imshow("Canny", dst);
//		// Copy edges to the images that will display the results in BGR
//
//		//cout << dst.channels() << endl;
//		//cvtColor(dst, cdst, CV_RGB2GRAY);
//		cvtColor(src, cdst, CV_BGRA2GRAY);
//		cdstP = cdst.clone();
//		// Standard Hough Line Transform
//		/*vector<Vec2f> lines; // will hold the results of the detection
//		HoughLines(dst, lines, 1, CV_PI / 180, 150, 0, 0); // runs the actual detection
//														   // Draw the lines
//		for (size_t i = 0; i < lines.size(); i++)
//		{
//			float rho = lines[i][0], theta = lines[i][1];
//			Point pt1, pt2;
//			double a = cos(theta), b = sin(theta);
//			double x0 = a*rho, y0 = b*rho;
//			pt1.x = cvRound(x0 + 1000 * (-b));
//			pt1.y = cvRound(y0 + 1000 * (a));
//			pt2.x = cvRound(x0 - 1000 * (-b));
//			pt2.y = cvRound(y0 - 1000 * (a));
//			line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA);
//		}
//		// Probabilistic Line Transform
//		vector<Vec4i> linesP; // will hold the results of the detection
//		HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 50, 10); // runs the actual detection
//															  // Draw the lines
//		for (size_t i = 0; i < linesP.size(); i++)
//		{
//			Vec4i l = linesP[i];
//			line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
//		}
//		// Show results
//		imshow("Source", src);
//		//imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);
//		imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
//		waitKey(2);
//
//		//cout << camera.get(cv::CAP_PROP_FPS) << endl;
//	}
//}*/
