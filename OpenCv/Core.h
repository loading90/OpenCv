#pragma once
#pragma region OpencvLib
#include "opencv2\core.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\videoio.hpp"
#include "opencv2\imgcodecs.hpp"
#pragma endregion
#pragma region IOLib
#include <iostream>

using std::cout;
using std::endl;
#pragma endregion
#pragma region UserLib
#include "Object.h"
#include "Rectangle.h"
#include "Mouse.hpp"
#pragma endregion

#define CORE_DEBUG

class Core;
extern Core core;

void ErrorExit(int code = -1);
cv::VideoCapture* OpenCamera(int nCamera = 1);

class Core
{
	cv::VideoCapture camera;

	cv::Mat inputImage;
	cv::Mat workingImage;

	std::string winName;

	const int maxLimit;
	int limit;

	std::vector<IObject*> objects;

	std::vector<cv::RotatedRect> foundObjects;

	void ConvertToBlkWht();
	void ConvertToCanny();
	void SetGrayBlurImage();

	void Process();
public:
	Core(unsigned defaultLimit = 100, unsigned maxLimit = 255);
	
	void SetCamera(cv::VideoCapture* camera);
	
	void Run();
};
