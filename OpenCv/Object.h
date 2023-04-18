#pragma once
#pragma region OpencvLib
#include "opencv2\core.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\videoio.hpp"
#include "opencv2\imgcodecs.hpp"
#pragma endregion
#include <vector>

typedef struct object
{
	//virtual bool Compare(cv::RotatedRect first, cv::RotatedRect second) = 0;
	virtual void CheckContours(std::vector<std::vector<cv::Point>>& contours, std::vector<cv::RotatedRect>& goodRect) = 0;
} IObject;
