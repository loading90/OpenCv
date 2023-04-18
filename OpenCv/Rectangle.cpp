#include "Rectangle.h"



Rectangle::Rectangle(unsigned plombSize, int percent)
	:percent(percent), plombSize(plombSize) {}

bool Compare(cv::RotatedRect first, cv::RotatedRect second)
{
	cv::Point2f tempFirst = first.size;
	cv::Point2f tempSecond = second.size;

	int squareFirst = tempFirst.x * tempFirst.y;
	int squareSecond = tempSecond.x * tempSecond.y;

	if (squareFirst > squareSecond)
		return true;
	return false;
}
void Rectangle::CheckContours(std::vector<std::vector<cv::Point>>& contours, std::vector<cv::RotatedRect>& goodRect)
{
	std::vector<cv::RotatedRect> minRect(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
	{
		// �������� ����� ��������������� ����� ������
		minRect[i] = minAreaRect(contours[i]);
	}

	std::sort(minRect.begin(), minRect.end(), Compare);

	int plombSquare = plombSize * plombSize;
	// �������� �������� � ������ ��������
	int plombSquarePlusPercent = plombSquare + plombSquare / 100 * percent;
	int plombSquareMinusPercent = plombSquare - plombSquare / 100 * percent;

	//std::vector<cv::RotatedRect> goodRect;

	// ��������� ������ �������� ����������������
	for (size_t i = 0; i < minRect.size(); i++)
	{
		// ���� ��� ��������� ��������� ����������������
		cv::Scalar color = cv::Scalar(255, 255, 0);
		// �������� ���� ���������������� ��� ��������
		cv::Point2f rect_points[4];
		minRect[i].points(rect_points);

		// ������� ������� �������� ����������������
		int temp = minRect[i].size.width * minRect[i].size.height;
		double dest = minRect[i].size.width - minRect[i].size.height;
		if (dest < 0.0)
			dest *= -1;
		// ��������� �������� �� �������, � ������ �� �������
		if ((temp <= plombSquarePlusPercent && temp >= plombSquareMinusPercent) && dest < 7.0)
		{
			goodRect.push_back(minRect[i]);
		}
	}

	//return &goodRect;
}
