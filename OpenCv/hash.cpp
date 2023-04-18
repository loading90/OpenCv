#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "PerceptualHashObjectDetector.hpp"
#include "Mouse.hpp"

using std::cout;
using std::endl;

int maink() {
	cout << "Start!" << endl;
	cv::VideoCapture camera(1);
	if (!camera.isOpened()) {
		// ���� �� ����� ������� ������, ������� ���������
		printf("Cannot open VideoCamera\n");
		return 0;
	}

	MouseHandler iMouse;
	// �������� ���� � �������� ���������� �����
	cvNamedWindow("result", CV_WINDOW_AUTOSIZE);
	iMouse.setWindows("result");

	const int HASH25_SIZE = 33554432;
	static unsigned char hash25[HASH25_SIZE]; // ������ ������������ �����
	static unsigned char meanData[256];
	memset(meanData, 0xFF, 256);
	
	std::vector<unsigned char*> hashes25;
	unsigned long trainHash = 0xFFFFFFFF; // ������������ ��� ��� ��������
	char nObj = 1; // ����� ������� ��� ��������
	int hammingDistance = 1; // ������������ ��������� ��������
	double scaleMin = 0.8; // ����������� ������� ���� ������ �� ��������������� ������� ��������������� �����
	double scaleMax = 1.2; // ������������ ������� ���� ������ �� ��������������� ������� ��������������� �����
	double scaleStep = 0.05; // ��� ��������
	double stepLength = 0.05; // ����� ���� ������ ������� � ��������� �� ��� ������ ��� ������

	const int MAX_HASH_BIT = 25; // ���������� ��� � ����
	cv::Mat inputImage; // ����������� �� �����

	camera >> inputImage; // �������� ����������� �� ������

	cv::Mat inputGrayImage; // ����� �����������
	cv::Mat imageIntegral(inputImage.cols, inputImage.rows, CV_32FC1); // ������������ �����������
	cv::Mat imageTrainHash; // �������� ��� �����������

	const int HASH_BOX = 5; // ������ � ������ ���� ���� 5 ��������
	const int TRAIN_BOX = 30; // ������ � ������ ���� ��� ����������� ���� � ��
							  //PerceptualHashObjectDetector::showHash32(trainHash, imageTrainHash, HASH_BOX, HASH_BOX, TRAIN_BOX, TRAIN_BOX, true);

	cv::Rect bbTrainHash; // ��������������� �����
	bbTrainHash.width = TRAIN_BOX;
	bbTrainHash.height = TRAIN_BOX;

	bool isRun = false;

	while (1) {
		camera >> inputImage;

		cvtColor(inputImage, inputGrayImage, CV_BGR2GRAY); // c������ �� �����������
		cv::integral(inputGrayImage, imageIntegral);

		// ������ �������� ��������������� �����
		iMouse.checkBb(inputImage);
		if (iMouse.boundingBox.x < 1) iMouse.boundingBox.x = 1;
		if (iMouse.boundingBox.y < 1) iMouse.boundingBox.y = 1;
		if (iMouse.boundingBox.x > inputImage.cols - TRAIN_BOX - iMouse.boundingBox.width) iMouse.boundingBox.x = inputImage.cols - TRAIN_BOX - iMouse.boundingBox.width;
		if (iMouse.boundingBox.y > inputImage.rows - TRAIN_BOX - iMouse.boundingBox.height) iMouse.boundingBox.y = inputImage.rows - TRAIN_BOX - iMouse.boundingBox.height;

		bbTrainHash.x = iMouse.boundingBox.x + iMouse.boundingBox.width;
		bbTrainHash.y = iMouse.boundingBox.y;
		// �������� ������������ ���
		trainHash = PerceptualHashObjectDetector::getHash32(inputGrayImage, iMouse.boundingBox, HASH_BOX, HASH_BOX);
		//PerceptualHashObjectDetector::showHash32(trainHash, imageTrainHash, HASH_BOX, HASH_BOX, TRAIN_BOX, TRAIN_BOX, true); // �������� ����������� ������������� ����

		// ������� �������� ������������� ���� �� �����
		char texthash[512];
		sprintf(texthash, "hash: %8.8X", trainHash);
		cv::putText(inputImage, texthash, cv::Point(iMouse.boundingBox.x, iMouse.boundingBox.y - 10), CV_FONT_HERSHEY_PLAIN, 0.9, cv::Scalar(0, 0, 255), 1, 8, 0);

		cv::Rect temp = iMouse.boundingBox;
		temp.height = inputImage.cols;
		cv::rectangle(inputImage, temp, cv::Scalar(0, 255, 0));
		cv::rectangle(inputImage, iMouse.boundingBox, cv::Scalar(255, 0, 0)); // �������� ��������������� �����
																			  // �������� ������������ ���
		cv::Mat roi(inputImage, bbTrainHash);
		imageTrainHash.copyTo(roi);
		roi.release();

		if (isRun) { // ���� �������� �������� ��������
			std::vector<cv::Rect> vBb; // ������ ���������� ��������������� ����� ��������
			std::vector<unsigned long> vHash; // ������ ���������� ���� ��������
			std::vector<cv::Scalar> vColor; // ������ ���������� ������ ����� ��������� ��������
			vColor.push_back(cv::Scalar(0, 255, 0));
			vColor.push_back(cv::Scalar(0, 255, 255));
			vColor.push_back(cv::Scalar(0, 0, 255));
			vColor.push_back(cv::Scalar(255, 255, 0));
			vColor.push_back(cv::Scalar(255, 255, 255));
			// ���� �������
			for (size_t i = 0; i < hashes25.size(); i++)
			{
				if (hashes25[i] != nullptr)
				{
					PerceptualHashObjectDetector::searhIntegralImage(imageIntegral,
						hash25, meanData,
						scaleMin, scaleMax, scaleStep, stepLength,
						iMouse.boundingBox.width, iMouse.boundingBox.height, vBb, vHash);
				}
			}
			/*PerceptualHashObjectDetector::searhIntegralImage(imageIntegral,
				hash25, meanData,
				scaleMin, scaleMax, scaleStep, stepLength,
				iMouse.boundingBox.width, iMouse.boundingBox.height, vBb, vHash);*/
			// ���������� ���������
			for (int i = 0; i < vBb.size(); ++i) {
				//int hashArrayData = hash25[vHash[i]];
				//int numColor = hashArrayData == 0x01 ? 1 : (hashArrayData == 0x02 ? 2 : (hashArrayData == 0x04 ? 3 : 4));
				cv::rectangle(inputImage, vBb[i], cv::Scalar(255, 255, 255)); // �������� ��������������� �����
				char texthash[512];
				sprintf(texthash, "%8.8X", vHash[i]);
				cv::putText(inputImage, texthash, cv::Point(vBb[i].x, vBb[i].y - 10), CV_FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 255, 255), 1, 8, 0);
			}
		}

		cv::imshow("result", inputImage);

		char symbol = cv::waitKey(20);
		if (symbol == '1' || symbol == '1') { // ������� ��� �������� ����� ������� 1
			nObj = 1; printf("set obj 1\n");
		}
		else
			if (symbol == '2' || symbol == '2') { // ������� ��� �������� ����� ������� 2
				nObj = 2; printf("set obj 2\n");
			}
			else
				if (symbol == '3' || symbol == '3') { // ������� ��� �������� ����� ������� 3
					nObj = 4; printf("set obj 3\n");
				}
				else
					if (symbol == 'B' || symbol == 'b') { // ����� �� ���������
						break;
					}
					else
						if (symbol == 'T' || symbol == 't') { // ��������� ��������� �������
							
							unsigned char* tempHash25 = new unsigned char[HASH25_SIZE];
							PerceptualHashObjectDetector::getNoise32(trainHash, hash25, nObj, hammingDistance, MAX_HASH_BIT);
							PerceptualHashObjectDetector::getNoise32(trainHash, tempHash25, nObj, hammingDistance, MAX_HASH_BIT);
							hashes25.push_back(tempHash25);
							printf("add hash: %8.8X\n", trainHash);
						}
						else
							if (symbol == 'R' || symbol == 'r') { // ��������� ��� ���������� �������� ��������
								isRun = !isRun;
								if (isRun) printf("hash detector on\n");
								else printf("hash detector off\n");
							}
							else
								if (symbol == 'C' || symbol == 'c') { // �������� ����
									for (int i = 0; i < hashes25.size(); i++)
									{
										if (hashes25[i] != nullptr)
										{
											delete[] hashes25[i];
										}
									}
									hashes25.clear();
									printf("clear hash\n");
								}
	}
	// ����������� ������
	inputImage.release();
	imageIntegral.release();
	imageTrainHash.release();
	return 0;
}
