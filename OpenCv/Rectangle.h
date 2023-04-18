#pragma once
#include <algorithm>
#include "Object.h"

static bool Compare(cv::RotatedRect first, cv::RotatedRect second);

class Rectangle :
	public IObject
{
	const int percent;
	unsigned plombSize;

public:
	Rectangle(unsigned plombSize = 30, int percent = 30);

	
	void CheckContours(std::vector<std::vector<cv::Point>>& contours, std::vector<cv::RotatedRect>& goodRect) override;
};

