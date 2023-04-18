#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

#include "PerceptualHashObjectDetector.hpp"
#include "Mouse.hpp"

using namespace std;

int mainSource() {
	std::cout << "enter camera number: " << std::endl;
	int nCamera = 0; // номер камеры
	std::cin >> nCamera;
	cv::VideoCapture camera(nCamera);
	if (!camera.isOpened()) {
		// если не можем открыть камеру, выводим сообщение
		printf("Cannot open initialize webcam or video file!\n");
		return 0;
	}

	MouseHandler iMouse;
	// создадим окно и поставим обработчик мышки
	cvNamedWindow("result", CV_WINDOW_AUTOSIZE);
	iMouse.setWindows("result");

	const int HASH25_SIZE = 33554432;
	static unsigned char hash25[HASH25_SIZE]; // массив перцептивных хэшей
	static unsigned char meanData[256];
	memset(meanData, 0xFF, 256);

	unsigned long trainHash = 0xFFFFFFFF; // перцептивный хэш для обучения
	char nObj = 1; // номер объекта для обучения
	int hammingDistance = 10; // максимальная дистнация хэмминга
	double scaleMin = 0.8; // минимальный машстаб окна поиска от первоначального размера ограничительной рамки
	double scaleMax = 1.2; // максимальный машстаб окна поиска от первоначального размера ограничительной рамки
	double scaleStep = 0.05; // шаг масштаба
	double stepLength = 0.05; // сдвиг окна поиска объекта в процентах от его ширины или высоты

	const int MAX_HASH_BIT = 5; // количество бит в хэше
	cv::Mat inputImage; // изображение на входе

	camera >> inputImage; // получаем изображение от камеры

	cv::Mat inputGrayImage; // серое изображение
	cv::Mat imageIntegral(inputImage.cols, inputImage.rows, CV_32FC1); // интегральное изображение
	cv::Mat imageTrainHash; // фрагмент для запоминания

	const int HASH_BOX = 5; // ширина и высота окна хэша 5 пикселей
	const int TRAIN_BOX = 30; // ширина и высота окна для отображения хэша в ЧБ
	//PerceptualHashObjectDetector::showHash32(trainHash, imageTrainHash, HASH_BOX, HASH_BOX, TRAIN_BOX, TRAIN_BOX, true);

	cv::Rect bbTrainHash; // ограничительная рамка
	bbTrainHash.width = TRAIN_BOX;
	bbTrainHash.height = TRAIN_BOX;

	bool isRun = false;

	while (1) {
		camera >> inputImage;

		cvtColor(inputImage, inputGrayImage, CV_BGR2GRAY); // cоздаем ЧБ изображение
		cv::integral(inputGrayImage, imageIntegral);

		// делаем проверку ограничительных рамок
		iMouse.checkBb(inputImage);
		if (iMouse.boundingBox.x < 1) iMouse.boundingBox.x = 1;
		if (iMouse.boundingBox.y < 1) iMouse.boundingBox.y = 1;
		if (iMouse.boundingBox.x > inputImage.cols - TRAIN_BOX - iMouse.boundingBox.width) iMouse.boundingBox.x = inputImage.cols - TRAIN_BOX - iMouse.boundingBox.width;
		if (iMouse.boundingBox.y > inputImage.rows - TRAIN_BOX - iMouse.boundingBox.height) iMouse.boundingBox.y = inputImage.rows - TRAIN_BOX - iMouse.boundingBox.height;

		bbTrainHash.x = iMouse.boundingBox.x + iMouse.boundingBox.width;
		bbTrainHash.y = iMouse.boundingBox.y;
		// получаем перцептивный хэш
		trainHash = PerceptualHashObjectDetector::getHash32(inputGrayImage, iMouse.boundingBox, HASH_BOX, HASH_BOX);
		//PerceptualHashObjectDetector::showHash32(trainHash, imageTrainHash, HASH_BOX, HASH_BOX, TRAIN_BOX, TRAIN_BOX, true); // получаем изображение перцептивного хэша

																															 // выводим значение перцептивного хэша на экран
		char texthash[512];
		sprintf(texthash, "hash: %8.8X", trainHash);
		cv::putText(inputImage, texthash, cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y - 10), CV_FONT_HERSHEY_PLAIN, 0.9, cv::Scalar(0, 0, 255), 1, 8, 0);

		cv::rectangle(inputImage, iMouse.boundingBox, cv::Scalar(255, 0, 0)); // нарисуем ограничительную рамку
																			  // нарисуем перцептивный хэш
		cv::Mat roi(inputImage, bbTrainHash);
		imageTrainHash.copyTo(roi);
		roi.release();

		if (isRun) { // если детектор объектов работает
			std::vector<cv::Rect> vBb; // вектор содержащий ограничительные рамки объектов
			std::vector<unsigned long> vHash; // вектор содержаший хэши объектов
			std::vector<cv::Scalar> vColor; // вектор содержащий разные цвета найденных объектов
			vColor.push_back(cv::Scalar(0, 255, 0));
			vColor.push_back(cv::Scalar(0, 255, 255));
			vColor.push_back(cv::Scalar(0, 0, 255));
			vColor.push_back(cv::Scalar(255, 255, 0));
			vColor.push_back(cv::Scalar(255, 255, 255));
			// ищем объекты
			PerceptualHashObjectDetector::searhIntegralImage(imageIntegral,
				hash25, meanData,
				scaleMin, scaleMax, scaleStep, stepLength,
				iMouse.boundingBox.width, iMouse.boundingBox.height, vBb, vHash);
			// отображаем найденное
			for (int i = 0; i < vBb.size(); ++i) {
				int hashArrayData = hash25[vHash[i]];
				int numColor = hashArrayData == 0x01 ? 1 : (hashArrayData == 0x02 ? 2 : (hashArrayData == 0x04 ? 3 : 4));
				cv::rectangle(inputImage, vBb[i], vColor[numColor]); // нарисуем ограничительную рамку
				char texthash[512];
				sprintf(texthash, "%8.8X", vHash[i]);
				cv::putText(inputImage, texthash, cv::Point(vBb[i].x, vBb[i].y - 10), CV_FONT_HERSHEY_PLAIN, 1.0, vColor[numColor], 1, 8, 0);
			}
		}

		cv::imshow("result", inputImage);

		char symbol = cv::waitKey(20);
		if (symbol == '1' || symbol == '1') { // выбрать для обучения номер объекта 1
			nObj = 1; printf("set obj 1\n");
		}
		else
			if (symbol == '2' || symbol == '2') { // выбрать для обучения номер объекта 2
				nObj = 2; printf("set obj 2\n");
			}
			else
				if (symbol == '3' || symbol == '3') { // выбрать для обучения номер объекта 3
					nObj = 3; printf("set obj 3\n");
				}
				else
					if (symbol == 'B' || symbol == 'b') { // выйти из программы
						break;
					}
					else
						if (symbol == 'T' || symbol == 't') { // запомнить обучающую выборку
							PerceptualHashObjectDetector::getNoise32(trainHash, hash25, nObj, hammingDistance, MAX_HASH_BIT);
							printf("add hash: %8.8X\n", trainHash);
						}
						else
							if (symbol == 'R' || symbol == 'r') { // запустить или остановить детектор объектов
								isRun = !isRun;
								if (isRun) printf("hash detector on\n");
								else printf("hash detector off\n");
							}
							else
								if (symbol == 'C' || symbol == 'c') { // очистить цели
									memset(hash25, 0, HASH25_SIZE);
									printf("clear hash\n");
								}
	}
	// освобождаем память
	inputImage.release();
	imageIntegral.release();
	imageTrainHash.release();
	return 0;
}

/*#define _CRT_SECURE_NO_WARNINGS
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

static void help()
{
cout << "\nThis program demonstrates the use of cv::CascadeClassifier class to detect objects (Face + eyes). You can use Haar or LBP features.\n"
"This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
"It's most known use is for faces.\n"
"Usage:\n"
"./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
"   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
"   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
"   [--try-flip]\n"
"   [filename|camera_index]\n\n"
"see facedetect.cmd for one call:\n"
"./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml\" --scale=1.3\n\n"
"During execution:\n\tHit any key to quit.\n"
"\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw(Mat& img, CascadeClassifier& cascade,
CascadeClassifier& nestedCascade,
double scale, bool tryflip);

string cascadeName;
string nestedCascadeName;

int main(int argc, const char** argv)
{
VideoCapture capture;
Mat frame, image;
string inputName;
bool tryflip;
CascadeClassifier cascade, nestedCascade;
double scale;

cv::CommandLineParser parser(argc, argv,
"{help h||}"
"{cascade|haarcascade_frontalface_alt.xml|}"
"{nested-cascade|haarcascade_eye_tree_eyeglasses.xml|}"
"{scale|1|}{try-flip||}{@filename||}"
);
if (parser.has("help"))
{
cout << parser.get<string>("cascade") << endl;
help();
system("pause");
return 0;
}
cascadeName = parser.get<string>("cascade");
nestedCascadeName = parser.get<string>("nested-cascade");
scale = parser.get<double>("scale");
if (scale < 1)
scale = 1;
tryflip = parser.has("try-flip");
inputName = parser.get<string>("@filename");
if (!parser.check())
{
parser.printErrors();
system("pause");
return 0;
}
if (!nestedCascade.load(nestedCascadeName))
cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
if (!cascade.load(cascadeName))
{
cerr << "ERROR: Could not load classifier cascade" << endl;
//help();
system("pause");
return -1;
}
if (inputName.empty() || (isdigit(inputName[0]) && inputName.size() == 1))
{
int camera = inputName.empty() ? 0 : inputName[0] - '0';
if (!capture.open(camera))
cout << "Capture from camera #" << camera << " didn't work" << endl;
}
else if (inputName.size())
{
image = imread(inputName, 1);
if (image.empty())
{
if (!capture.open(inputName))
cout << "Could not read " << inputName << endl;
}
}
else
{
image = imread("../data/lena.jpg", 1);
if (image.empty()) cout << "Couldn't read ../data/lena.jpg" << endl;
}

if (capture.isOpened())
{
cout << "Video capturing has been started ..." << endl;

for (;;)
{
capture >> frame;
if (frame.empty())
break;

Mat frame1 = frame.clone();
detectAndDraw(frame1, cascade, nestedCascade, scale, tryflip);

char c = (char)waitKey(10);
if (c == 27 || c == 'q' || c == 'Q')
break;
}
}
else
{
cout << "Detecting face(s) in " << inputName << endl;
if (!image.empty())
{
detectAndDraw(image, cascade, nestedCascade, scale, tryflip);
waitKey(0);
}
else if (!inputName.empty())
{
// assume it is a text file containing the
//list of the image filenames to be processed - one per line
FILE* f = fopen(inputName.c_str(), "rt");
if (f)
{
char buf[1000 + 1];
while (fgets(buf, 1000, f))
{
int len = (int)strlen(buf);
while (len > 0 && isspace(buf[len - 1]))
len--;
buf[len] = '\0';
cout << "file " << buf << endl;
image = imread(buf, 1);
if (!image.empty())
{
detectAndDraw(image, cascade, nestedCascade, scale, tryflip);
char c = (char)waitKey(0);
if (c == 27 || c == 'q' || c == 'Q')
break;
}
else
{
cerr << "Aw snap, couldn't read image " << buf << endl;
}
}
fclose(f);
}
}
}

return 0;
}

void detectAndDraw(Mat& img, CascadeClassifier& cascade,
CascadeClassifier& nestedCascade,
double scale, bool tryflip)
{
double t = 0;
vector<Rect> faces, faces2;
const static Scalar colors[] =
{
Scalar(255,0,0),
Scalar(255,128,0),
Scalar(255,255,0),
Scalar(0,255,0),
Scalar(0,128,255),
Scalar(0,255,255),
Scalar(0,0,255),
Scalar(255,0,255)
};
Mat gray, smallImg;

cvtColor(img, gray, COLOR_BGR2GRAY);
double fx = 1 / scale;
resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
equalizeHist(smallImg, smallImg);

t = (double)getTickCount();
cascade.detectMultiScale(smallImg, faces,
1.1, 2, 0
//|CASCADE_FIND_BIGGEST_OBJECT
//|CASCADE_DO_ROUGH_SEARCH
| CASCADE_SCALE_IMAGE,
Size(30, 30));
if (tryflip)
{
flip(smallImg, smallImg, 1);
cascade.detectMultiScale(smallImg, faces2,
1.1, 2, 0
//|CASCADE_FIND_BIGGEST_OBJECT
//|CASCADE_DO_ROUGH_SEARCH
| CASCADE_SCALE_IMAGE,
Size(30, 30));
for (vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r)
{
faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
}
}
t = (double)getTickCount() - t;
printf("detection time = %g ms\n", t * 1000 / getTickFrequency());
for (size_t i = 0; i < faces.size(); i++)
{
Rect r = faces[i];
Mat smallImgROI;
vector<Rect> nestedObjects;
Point center;
Scalar color = colors[i % 8];
int radius;

double aspect_ratio = (double)r.width / r.height;
if (0.75 < aspect_ratio && aspect_ratio < 1.3)
{
center.x = cvRound((r.x + r.width*0.5)*scale);
center.y = cvRound((r.y + r.height*0.5)*scale);
radius = cvRound((r.width + r.height)*0.25*scale);
circle(img, center, radius, color, 3, 8, 0);
}
else
rectangle(img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
Point(cvRound((r.x + r.width - 1)*scale), cvRound((r.y + r.height - 1)*scale)),
color, 3, 8, 0);
if (nestedCascade.empty())
continue;
smallImgROI = smallImg(r);
nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
1.1, 2, 0
//|CASCADE_FIND_BIGGEST_OBJECT
//|CASCADE_DO_ROUGH_SEARCH
//|CASCADE_DO_CANNY_PRUNING
| CASCADE_SCALE_IMAGE,
Size(30, 30));
for (size_t j = 0; j < nestedObjects.size(); j++)
{
Rect nr = nestedObjects[j];
center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
radius = cvRound((nr.width + nr.height)*0.25*scale);
circle(img, center, radius, color, 3, 8, 0);
}
}
imshow("result", img);
}*/
/*#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"

#include <vector>

using namespace std;
using namespace cv;

void readme(string &message)
{
cout << message << endl;
}

int main(int argc, char** argv)
{
if (argc != 2)
{
string message = "Использование: ./cv_test <img_object>";
readme(message);
return -1;
}

VideoCapture cap(0); // Открыть камеру (устройство 1). Для открытия встроенной камеры вызывать 0 устройство.

if (!cap.isOpened())  //  Проверка корректности отработки
{
string message = "Проверьте камеру или укажите другой номер устройства в коде";
readme(message);
return -1;
}

Mat img_object = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

for (;;)
{
Mat frame;
cap >> frame; // Получить очередной фрейм из камеры

Mat img_scene = frame;

if (!img_object.data || !img_scene.data) // Проверка наличия информации в матрице изображения
{

string message = " Ошибка чтения ";
readme(message);
}

int minHessian = 400;

SurfFeatureDetector detector( minHessian );

std::vector<KeyPoint> keypoints_object, keypoints_scene;

detector.detect( img_object, keypoints_object );
detector.detect( img_scene, keypoints_scene );
}

return 0;
}*/
/*#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace cv;
using namespace cv::xfeatures2d;
void readme();
// @function main
int main(int argc, char** argv)
{
if (argc != 3)
{
readme(); return -1;
}
Mat img_object = imread("lena-fragment.jpg", IMREAD_GRAYSCALE);
Mat img_scene = imread("lena.jpg", IMREAD_GRAYSCALE);
if (!img_object.data || !img_scene.data)
{
std::cout << " --(!) Error reading images " << std::endl; return -1;
}
//-- Step 1: Detect the keypoints and extract descriptors using SURF
int minHessian = 400;
Ptr<SURF> detector = SURF::create(minHessian);
std::vector<KeyPoint> keypoints_object, keypoints_scene;
Mat descriptors_object, descriptors_scene;
detector->detectAndCompute(img_object, Mat(), keypoints_object, descriptors_object);
detector->detectAndCompute(img_scene, Mat(), keypoints_scene, descriptors_scene);
//-- Step 2: Matching descriptor vectors using FLANN matcher
FlannBasedMatcher matcher;
std::vector< DMatch > matches;
matcher.match(descriptors_object, descriptors_scene, matches);
double max_dist = 0; double min_dist = 100;
//-- Quick calculation of max and min distances between keypoints
for (int i = 0; i < descriptors_object.rows; i++)
{
double dist = matches[i].distance;
if (dist < min_dist) min_dist = dist;
if (dist > max_dist) max_dist = dist;
}
printf("-- Max dist : %f \n", max_dist);
printf("-- Min dist : %f \n", min_dist);
//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
std::vector< DMatch > good_matches;
for (int i = 0; i < descriptors_object.rows; i++)
{
if (matches[i].distance < 3 * min_dist)
{
good_matches.push_back(matches[i]);
}
}
Mat img_matches;
drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
//-- Localize the object
std::vector<Point2f> obj;
std::vector<Point2f> scene;
for (size_t i = 0; i < good_matches.size(); i++)
{
//-- Get the keypoints from the good matches
obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
}
Mat H = findHomography(obj, scene, RANSAC);
//-- Get the corners from the image_1 ( the object to be "detected" )
std::vector<Point2f> obj_corners(4);
obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(img_object.cols, 0);
obj_corners[2] = cvPoint(img_object.cols, img_object.rows); obj_corners[3] = cvPoint(0, img_object.rows);
std::vector<Point2f> scene_corners(4);
perspectiveTransform(obj_corners, scene_corners, H);
//-- Draw lines between the corners (the mapped object in the scene - image_2 )
line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
//-- Show detected matches
imshow("Good Matches & Object detection", img_matches);

waitKey(0);
return 0;
}

void readme()
{
std::cout << " Usage: ./SURF_descriptor <img1> <img2>" << std::endl;
}*/
