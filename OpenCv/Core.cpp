#include "Core.h"

void ErrorExit(int code)
{
	system("pause");
	exit(code);
}
cv::VideoCapture* OpenCamera(int nCamera)
{
	try
	{
		cv::VideoCapture* camera = new cv::VideoCapture(nCamera);
		if (camera->isOpened())
			return camera;
		else
			cout << "Error to open camera, number camera: " << nCamera << endl;
		ErrorExit();

	}
	catch (cv::Exception& exp)
	{
		cout << exp.msg << " in file: " << exp.file << ", code: " << exp.code << endl;
		ErrorExit();
	}
	return nullptr;
}

Core core;

Core::Core(unsigned defaultLimit, unsigned maxLimit)
	:limit(defaultLimit), maxLimit(maxLimit)
{
	this->camera = *OpenCamera();
	this->winName = "Camera";
	cv::namedWindow(this->winName.data(), CV_WINDOW_AUTOSIZE);
	this->objects.push_back(new Rectangle);
}

void Core::ConvertToBlkWht()
{
	for (int y = 0; y < workingImage.rows; y++)
	{
		for (int x = 0; x < workingImage.cols; x++)
		{
			cv::Vec3b pixel = workingImage.at<cv::Vec3b>(cv::Point(x, y));
			int b, g, r;
			b = pixel[0];
			g = pixel[1];
			r = pixel[2];

			if (b > limit && g > limit && r > limit)
			{
				b = 255;
				g = 255;
				r = 255;
			}
			else
			{
				b = 0;
				g = 0;
				r = 0;
			}

			pixel[0] = b;
			pixel[1] = g;
			pixel[2] = r;

			workingImage.at<cv::Vec3b>(cv::Point(x, y)) = pixel;
		}
	}
	#ifdef CORE_DEBUG
		cv::imshow("Snapshot", workingImage);
	#endif // CORE_DEBUG
}
void Core::ConvertToCanny()
{
	cv::Canny(workingImage, workingImage, 100, 100 * 2);
}
void Core::SetGrayBlurImage()
{
	workingImage.convertTo(workingImage, CV_8U);

	cv::cvtColor(workingImage, workingImage, cv::COLOR_BGR2GRAY);
	cv::blur(workingImage, workingImage, cv::Size(3, 3));

}
void Core::Process()
{
	ConvertToBlkWht();
	SetGrayBlurImage();
	ConvertToCanny();

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(workingImage, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	/* Данный блок кода работает только при одном объекте */
	std::vector<cv::RotatedRect> goodMatches;
	this->objects[0]->CheckContours(contours, goodMatches);

	/* -------------------------------------------------- */

	/* Очищаем workingImage */
	workingImage.release();

	/* Отрисовка найденых объектов, отрисовка происходит на входящем изображении */

	cv::Point2f rect_points[4];
	cv::Scalar color = cv::Scalar(255, 255, 0);

	if (goodMatches.size() != 0)
	{
		for (size_t i = 0; i < goodMatches.size(); i++)
		{
			goodMatches[i].points(rect_points);
			for (int j = 0; j < 4; j++)
			{
				line(inputImage, rect_points[j], rect_points[(j + 1) % 4], color, 2);
			}
		}
	}
}

void Core::SetCamera(cv::VideoCapture* camera)
{
	if (camera != nullptr)
		this->camera = *camera;
	else
		throw new std::exception_ptr;
}

void Core::Run()
{
	while (true)
	{
		camera >> inputImage;
		char sym = cv::waitKey(2);

#ifdef CORE_DEBUG
		if (sym == 'd')
		{
			//cv::RotateFlags rotate;
			int rotate = 0;
			bool rotated = false;
			while (true)
			{
				camera >> inputImage;
				char sym = cv::waitKey(2);

				cv::circle(inputImage, cv::Point2f(1.0f / 4 * inputImage.cols, 1.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
				cv::circle(inputImage, cv::Point2f(3.0f / 4 * inputImage.cols, 1.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
				cv::circle(inputImage, cv::Point2f(1.0f / 2 * inputImage.cols, 1.0f / 2 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
				cv::circle(inputImage, cv::Point2f(1.0f / 4 * inputImage.cols, 2.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);
				cv::circle(inputImage, cv::Point2f(3.0f / 4 * inputImage.cols, 2.0f / 3 * inputImage.rows), 2, cv::Scalar(0, 0, 0), 3);

				if (sym == 'e')
					break;
				
				if (sym == 'r')
				{
					if (rotate == 3)
						rotate = 0;
					else
						++rotate;
				}
				switch (rotate)
				{
				case 1:
					cv::rotate(inputImage, inputImage, cv::ROTATE_90_CLOCKWISE);
					break;
				case 2:
					cv::rotate(inputImage, inputImage, cv::ROTATE_180);
					break;
				case 3:
					cv::rotate(inputImage, inputImage, cv::ROTATE_90_COUNTERCLOCKWISE);
					break;
				}

				cv::imshow(winName, inputImage);
			}
		}
#endif // CORE_DEBUG

		if (sym == 's')
		{
			while (true)
			{
				camera >> inputImage;

				workingImage = inputImage.clone();

				cv::createTrackbar("Limit:", winName, &limit, maxLimit);

				Process();
				cv::imshow(winName, inputImage);

				sym = cv::waitKey(2);
				if (sym == 'e')
					break;
			}
		}
		cv::imshow(winName, inputImage);
	}
}
