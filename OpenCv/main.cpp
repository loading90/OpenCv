#include "Core.h"

int main()
{
	core.Run();

	return 0;
}

//#include <iostream>
//
//#include "Mouse.hpp"
//
//#include "opencv2\core.hpp"
//#include "opencv2\opencv.hpp"
//#include "opencv2\highgui.hpp"
//#include "opencv2\videoio.hpp"
//#include "opencv2\imgcodecs.hpp"
//
//using std::cout;
//using std::endl;
//
//cv::Mat inputImage;
//cv::Mat workingImage;
//
//cv::Point2f plomb;
//
//std::string winName = "Camera";
//
//const int maxThresh = 255;
//int thresh = 100;
//
//void ErrorExit(int code = -1)
//{
//	system("pause");
//	exit(code);
//}
//cv::VideoCapture* OpenCamera(int nCamera = 1)
//{
//	try
//	{
//		cv::VideoCapture* camera = new cv::VideoCapture(nCamera);
//		if (!camera->isOpened())
//		{
//			cout << "Error to open camera with id " << nCamera << "!" << endl;
//		}
//
//		return camera;
//	}
//	catch (cv::Exception& exp)
//	{
//		cout << exp.msg << " in file: " << exp.file << ", code: " << exp.code << endl;
//		ErrorExit();
//	}
//	return nullptr;
//}
//void SetGrayBlurImage(cv::Mat& image)
//{
//	image.convertTo(image, CV_8U);
//
//	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
//	cv::blur(image, image, cv::Size(3, 3));
//}
//bool Compare(cv::RotatedRect first, cv::RotatedRect second)
//{
//	cv::Point2f tempFirst = first.size;
//	cv::Point2f tempSecond = second.size;
//
//	int squareFirst = tempFirst.x * tempFirst.y;
//	int squareSecond = tempSecond.x * tempSecond.y;
//
//	if (squareFirst > squareSecond)
//		return true;
//	return false;
//}
//
//void ConvertToBlkWht()
//{
//	for (int y = 0; y < workingImage.rows; y++)
//	{
//		for (int x = 0; x < workingImage.cols; x++)
//		{
//			cv::Vec3b pixel = workingImage.at<cv::Vec3b>(cv::Point(x, y));
//			int b, g, r;
//			b = pixel[0];
//			g = pixel[1];
//			r = pixel[2];
//
//			if (b > thresh && g > thresh && r > thresh)
//			{
//				b = 255;
//				g = 255;
//				r = 255;
//			}
//			else
//			{
//				b = 0;
//				g = 0;
//				r = 0;
//			}
//
//			pixel[0] = b;
//			pixel[1] = g;
//			pixel[2] = r;
//
//			workingImage.at<cv::Vec3b>(cv::Point(x, y)) = pixel;
//		}
//	}
//	cv::imshow("Snapshot", workingImage);
//}
//
//void Process(int, void*)
//{
//	/**///Подготавливаем изображение, находим контуры
//	std::vector<std::vector<cv::Point>> contours;
//	cv::Canny(workingImage, workingImage, thresh, thresh * 2);
//
//	cv::findContours(workingImage, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
//	/**/
//
//	// Коллекция четырехугольников
//	std::vector<cv::RotatedRect> minRect(contours.size());
//	// Коллекция окружностей
//	std::vector<cv::RotatedRect> minEllipse(contours.size());
//	// Проверяем на соответсвие все контуры
//	for (size_t i = 0; i < contours.size(); i++)
//	{
//		// Пытаемся найти четырехугольник через контур
//		minRect[i] = minAreaRect(contours[i]);
//		if (contours[i].size() > 5)
//		{
//			// Пытаемся найти окуржность через контур
//			minEllipse[i] = fitEllipse(contours[i]);
//		}
//	}
//	//cv::Mat drawing = cv::Mat::zeros(workingImage.size(), CV_8UC3);
//	
//	// Функция сортировки
//	std::sort(minRect.begin(), minRect.end(), Compare);
//	
//	/*cv::Point2f pointMaxSquare = minRect[0].size;
//	int maxSquare = pointMaxSquare.x * pointMaxSquare.y;*/
//
//	// Процент расхождения
//	const int percent = 30;
//	// Площадь эталонного квадрата
//	int plombSquare = plomb.x * plomb.y;
//	// Диапазон площадей с учетом процента
//	int plombSquarePlusPercent = plombSquare + plombSquare / 100 * percent;
//	int plombSquareMinusPercent = plombSquare - plombSquare / 100 * percent;
//
//	// Проверяем каджый найденый четырехугольника
//	for (size_t i = 0; i < minRect.size(); i++)
//	{
//		// Цвет для выделения найденого четырехугольника
//		cv::Scalar color = cv::Scalar(255,255,0);
//		// Получаем углы четырехугольника для отисовки
//		cv::Point2f rect_points[4];
//		minRect[i].points(rect_points);
//
//		// Счиатем площадь текущего четырехугольника
//		int temp = minRect[i].size.width * minRect[i].size.height;
//		double dest = minRect[i].size.width - minRect[i].size.height;
//		if (dest < 0.0)
//			dest *= -1;
//		// Проверяем подходит ли площадь, и равные ли стороны
//		if ((temp <= plombSquarePlusPercent && temp >= plombSquareMinusPercent) && dest < 7.0)
//		{
//			// Отрисовываем четырехугольник
//			for (int j = 0; j < 4; j++)
//			{
//				line(inputImage, rect_points[j], rect_points[(j + 1) % 4], color, 2);
//			}
//		}
//	}
//}
//
//int main()
//{
//	cv::VideoCapture camera = *OpenCamera();
//	cv::namedWindow(winName.data(), CV_WINDOW_AUTOSIZE);
//	
//	MouseHandler iMouse;
//	iMouse.setWindows(winName.data());
//
//	cv::Vec3b color;
//	cv::Point colorPoint;
//
//	//camera.read(inputImage);
//
//	/*while (true)
//	{
//		camera.read(inputImage);
//
//		iMouse.checkBb(inputImage);
//		if (iMouse.boundingBox.x < 1) iMouse.boundingBox.x = 1;
//		if (iMouse.boundingBox.y < 1) iMouse.boundingBox.y = 1;
//
//		cv::Scalar colorFrame(25, 0, 0);
//
//		cv::line(inputImage, cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y), cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y + iMouse.boundingBox.height), colorFrame, 2);
//		cv::line(inputImage, cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y), cv::Point(iMouse.boundingBox.x + iMouse.boundingBox.width, iMouse.boundingBox.y), colorFrame, 2);
//		cv::line(inputImage, cv::Point(iMouse.boundingBox.x + iMouse.boundingBox.width, iMouse.boundingBox.y), cv::Point(iMouse.boundingBox.x + iMouse.boundingBox.width, iMouse.boundingBox.y + iMouse.boundingBox.height), colorFrame, 2);
//		cv::line(inputImage, cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y + iMouse.boundingBox.height), cv::Point(iMouse.boundingBox.x + iMouse.boundingBox.width, iMouse.boundingBox.y + iMouse.boundingBox.height), colorFrame, 2);
//
//		char sym = cv::waitKey(2);
//		if (sym == 'm' || sym == 'M')
//		{
//			plomb.x = iMouse.boundingBox.width;
//			plomb.y = iMouse.boundingBox.height;
//			cout << plomb.x << " " << plomb.y << endl;
//			break;
//		}
//		/*if (sym == 'c' || sym == 'C')
//		{
//			snapshot = inputImage.clone();
//
//			for (int y = 0; y < snapshot.rows; y++)
//			{
//				for (int x = 0; x < snapshot.cols; x++)
//				{
//					cv::Vec3b pixel = snapshot.at<cv::Vec3b>(cv::Point(x, y));
//					int b, g, r;
//					b = pixel[0];
//					g = pixel[1];
//					r = pixel[2];
//
//					if (b > 100 && g > 100 && r > 100)
//					{
//						b = 255;
//						g = 255;
//						r = 255;
//					}
//					else
//					{
//						b = 0;
//						g = 0;
//						r = 0;
//					}
//
//					pixel[0] = b;
//					pixel[1] = g;
//					pixel[2] = r;
//
//					snapshot.at<cv::Vec3b>(cv::Point(x, y)) = pixel;
//				}
//			}
//
//			cv::imshow("Snapshot", snapshot);
//
//			/cv::Vec3b pixel = snapshot.at<cv::Vec3b>(cv::Point(colorPoint.x, colorPoint.y));
//			int b, g, r;
//			b = pixel[0];
//			g = pixel[1];
//			r = pixel[2];
//			std::string rgbText = "[" + std::to_string(r) + ", " + std::to_string(g)
//				+ ", " + std::to_string(b) + "]";
//
//			cout << rgbText << endl;
//
//			//break;
//		}
//		cv::imshow(winName, inputImage);
//	}*/
//
//	plomb.x = 30;
//	plomb.y = 30;
//
//	try
//	{
//		while (true)
//		{
//			camera >> inputImage;
//			char sym = cv::waitKey(2);
//
//			if (sym == 'i')
//			{
//				while (true)
//				{
//					camera >> inputImage;
//					char sym = cv::waitKey(2);
//
//					cv::circle(inputImage, cv::Point2f(1.0f / 4 * inputImage.cols, 1.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
//					cv::circle(inputImage, cv::Point2f(3.0f / 4 * inputImage.cols, 1.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
//					cv::circle(inputImage, cv::Point2f(1.0f / 2 * inputImage.cols, 1.0f / 2 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
//					cv::circle(inputImage, cv::Point2f(1.0f / 4 * inputImage.cols, 2.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
//					cv::circle(inputImage, cv::Point2f(3.0f / 4 * inputImage.cols, 2.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
//
//					if (sym == 'e')
//						break;
//
//					cv::rotate(inputImage,inputImage, cv::ROTATE_90_CLOCKWISE);
//
//					cv::imshow(winName, inputImage);
//				}
//			}
//
//			if (sym == 's' || sym == 'S')
//			{
//				while (true)
//				{
//					camera >> inputImage;
//
//					workingImage = inputImage.clone();
//					ConvertToBlkWht();
//					SetGrayBlurImage(workingImage);
//
//					cv::createTrackbar("Thresh:", winName, &thresh, maxThresh, Process);
//
//					Process(0, 0);
//					cv::imshow(winName, inputImage);
//
//					sym = cv::waitKey(2);
//					if (sym == 'e' || sym == 'E')
//						break;
//				}
//			}
//			cv::imshow(winName, inputImage);
//		}
//	}
//	catch (const std::exception& exp)
//	{
//		cout << exp.what() << endl;
//		system("pause");
//	}
//	
//
//	/*while (true)
//	{
//		cv::Mat img;
//		camera >> img;
//
//		cv::Mat snapshot = cv::Mat(img.size(), CV_8UC3, cv::Scalar(0, 0, 0));
//
//		for (int y = 50; y < snapshot.rows - 50; y++)
//		{
//			for (int x = 50; x<snapshot.cols - 50; x++)
//			{
//				// get pixel
//				cv::Vec3b color = snapshot.at<cv::Vec3b>(cv::Point(x, y));
//
//				color = cv::Vec3b(255, 128, 128);
//
//				// set pixel
//				snapshot.at<cv::Vec3b>(cv::Point(x, y)) = color;
//			}
//		}
//
//		cv::imshow("test", snapshot);
//		cv::waitKey(1000);
//	}*/
//
//	return 0;
//}