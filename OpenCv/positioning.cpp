#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using std::cout;
using std::endl;

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 1.0f;

/*cv::VideoCapture* OpenCamera(int numCamera = 1)
{
	cv::VideoCapture* camera = new cv::VideoCapture(numCamera);
	if (!camera->isOpened())
	{
		cout << "Error!" << endl;
		system("pause");
		exit(-1);
	}

	return camera;
}*/

#pragma region ORB
void OrbCompute(cv::Mat& imageGray, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors)
{
	static cv::Ptr<cv::Feature2D> orb = cv::ORB::create(MAX_FEATURES);

	orb->detectAndCompute(imageGray, cv::Mat(), keypoints, descriptors);
}
std::vector<cv::DMatch> MatchingBrute(cv::Mat& descriptorsObject, cv::Mat& descriptorsScene)
{
	static cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");

	std::vector<cv::DMatch> matches;
	matcher->match(descriptorsObject, descriptorsScene, matches, cv::Mat());

	std::sort(matches.begin(), matches.end());

	return matches;
}
void CalculateGoodMatches(std::vector<cv::DMatch>& matches)
{
	const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
	matches.erase(matches.begin() + numGoodMatches, matches.end());
}

void Handler(cv::Mat& scene, cv::Mat& object, cv::Mat& descriptorsObject, std::vector<cv::KeyPoint>& keypointsObject)
{
	cv::Mat sceneGray;
	cv::cvtColor(scene, sceneGray, CV_BGR2GRAY);

	std::vector<cv::KeyPoint> keypointsScene;
	cv::Mat descriptorsScene;

	unsigned long timer = time(NULL);
	OrbCompute(sceneGray, keypointsScene, descriptorsScene);

	std::vector<cv::DMatch> matches = MatchingBrute(descriptorsObject, descriptorsScene);
	CalculateGoodMatches(matches);

	cv::Mat img_matches = scene;
	cv::Mat h;

	//-- Нарисовать хорошие матчи
	/*drawMatches(object, keypointsObject, scene, keypointsScene,
	good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
	vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	*/
	//-- Локализация объектов
	std::vector<cv::Point2f> obj;
	std::vector<cv::Point2f> sc;

	for (int i = 0; i < matches.size(); i++)
	{
		obj.push_back(keypointsObject[matches[i].queryIdx].pt);
		sc.push_back(keypointsScene[matches[i].trainIdx].pt);
	}

	for (int i = 0; i < obj.size(); i++)
	{
		cv::circle(img_matches, sc[i], 2, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
	}

	h = cv::findHomography(obj, sc, CV_RANSAC);

	//-- Получить "углы" изображения с целевым объектом
	std::vector<cv::Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0);							obj_corners[1] = cvPoint(object.cols, 0);
	obj_corners[2] = cvPoint(object.cols, object.rows);		obj_corners[3] = cvPoint(0, object.rows);
	std::vector<cv::Point2f> scene_corners(4);

	//-- Отобразить углы целевого объекта, используя найденное преобразование, на сцену
	perspectiveTransform(obj_corners, scene_corners, h);

	//-- Соеденить отображенные углы
	line(img_matches, scene_corners[0] + cv::Point2f(object.cols, 0), scene_corners[1] + cv::Point2f(object.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[1] + cv::Point2f(object.cols, 0), scene_corners[2] + cv::Point2f(object.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[2] + cv::Point2f(object.cols, 0), scene_corners[3] + cv::Point2f(object.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[3] + cv::Point2f(object.cols, 0), scene_corners[0] + cv::Point2f(object.cols, 0), cv::Scalar(0, 255, 0), 4);

	cout << time(NULL) - timer << endl;
	//-- Show detected matches
	imshow("Result", img_matches);
}

int mainPosOrb()
{
	/*cv::VideoCapture* camera = OpenCamera();

	cv::Mat scene;
	cv::Mat object;

	std::string cameraWin("Camera"), resultWin("Result");

	cvNamedWindow(cameraWin.data(), CV_WINDOW_AUTOSIZE);
	cvNamedWindow(resultWin.data(), CV_WINDOW_AUTOSIZE);

	object = cv::imread("plomb_min.jpg");

	cv::Mat objectGray;
	cv::cvtColor(object, objectGray, CV_BGR2GRAY);

	std::vector<cv::KeyPoint> keypointsObject;
	cv::Mat descriptorsObject;

	OrbCompute(objectGray, keypointsObject, descriptorsObject);

	while (true)
	{
		*camera >> scene;

		cv::imshow(cameraWin, scene);

		char sym = cv::waitKey(2);
		if (sym == 's' || sym == 'S')
		{
			Handler(scene, object, descriptorsObject, keypointsObject);
		}
	}*/

	return 0;
}
#pragma endregion

namespace pos
{
	class vector
	{
	public:
		cv::Vec4i line;
		double x, y;

		vector() : vector(0, 0) {}
		vector(double x, double y) : x(x), y(y) {}
		vector(cv::Vec4i line)
		{
			x = line[0] - line[2];
			y = line[1] - line[3];
			this->line = line;
		}
		vector(double x1, double y1, double x2, double y2)
		{
			this->x = x1 - x2;
			this->y = y1 - y2;
		}
	};

	class Line
	{
	public:
		cv::Vec4i line;
		bool compare;

		Line(cv::Vec4i line) : line(line), compare(true) {}
	};
}

/* ln - line source, lc - line compare*/
bool IsParallel(cv::Vec4i ls, cv::Vec4i lc)
{
	pos::vector sourceVector(ls);
	pos::vector compareVector(lc);

	double scalar = sourceVector.x * compareVector.x + sourceVector.y * compareVector.y;

	double moduleSource = sqrt(sourceVector.x * sourceVector.x + sourceVector.y * sourceVector.y);
	double moduleCompare = sqrt(compareVector.x * compareVector.x + compareVector.y * compareVector.y);;

	double deviation = scalar / (moduleSource * moduleCompare);
	//double cosine = 0.9994;
	double cosine = 0.015;

	if (deviation < 0.0)
		deviation *= -1;

	if (cosine >= deviation)
	{
		double distance = sqrt((sourceVector.x - compareVector.x) * (sourceVector.x - compareVector.x) +
			(sourceVector.y - compareVector.y)* (sourceVector.y - compareVector.y));
		if (distance >= 100.0 && distance <= 116.0)
		{
			//cout << deviation << "   " << distance << endl;
			return true;
		}
	}

	return false;
}
/*
int main()
{
	cv::VideoCapture* camera = OpenCamera();

	std::string cameraName("Camera"), cannyName("Canny"), grayName("Gray");

	cv::Mat inputImage;
	cv::Mat cannyImage;
	cv::Mat grayImage;
	cv::Mat notSort;

	//cvNamedWindow(cameraName.data(), CV_WINDOW_AUTOSIZE);
	//cvNamedWindow(cannyName.data(), CV_WINDOW_AUTOSIZE);
	cvNamedWindow(grayName.data(), CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("Not sort", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		*camera >> inputImage;
		//cv::imshow(cameraName, inputImage);

		cv::Canny(inputImage, cannyImage, 50, 200, 3);
		//cv::imshow(cannyName, cannyImage);

		cv::waitKey(2);

		cv::cvtColor(inputImage, grayImage, CV_BGRA2GRAY);

		std::vector<cv::Vec4i> lines;
		cv::HoughLinesP(cannyImage, lines, 1, CV_PI / 180, 50, 50, 10);

		notSort = grayImage.clone();
		for (size_t i = 0; i < lines.size(); i++)
		{
			cv::Vec4i l = lines[i]; // Получаем линию
			cv::line(notSort, cvPoint(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
			//cv::line(grayImage, 1-Точка, 2-Точка, Цвет, Далее вид линии:  3, cv::LINE_AA);
		}
		//cv::imshow("Not sort", notSort);




		bool compare = false;
		std::vector<cv::Vec4i> goodLines;
		std::vector<pos::Line> compareLines;

		for (auto i = lines.begin(), end = lines.end(); i != end;)
		{
			compareLines.push_back(*i);
			++i;
		}

		for (auto i = compareLines.begin(), end = compareLines.end(); i != end;)
		{
			if (!compare || !(*i).compare)
			{
				compareLines.erase(i);
				i = compareLines.begin();
				end = compareLines.end();
			}
			compare = false;
			if (i != end)
			{
				for (auto j = i + 1; j != end;)
				{
					if ((*j).compare)
					{
						if (IsParallel((*i).line, (*j).line))
						{
							goodLines.push_back((*i).line);
							goodLines.push_back((*j).line);
							(*j).compare = false;
							i = compareLines.erase(i);
							end = compareLines.end();

							compare = true;

							break;
						}
						else
							++j;
					}
					else
						++j;
				}
			}
			else
				break;
		}

		for (size_t i = 0; i < goodLines.size(); i++)
		{
			cv::Vec4i l = goodLines[i];
			cv::line(grayImage, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
		}
		
		cv::imshow(grayName, grayImage);
		//char symbol = cv::waitKey(2);
		//while (symbol != 's')
		{
			//cv::imshow(cameraName, inputImage);
			//symbol = cv::waitKey(2);
		}
	}

	return 0;
}*/
