#pragma once

#include <opencv2/opencv.hpp>
#include <qmessagebox.h>

using namespace cv;
using namespace std;

QString image_to_str(Mat);
Mat convertToRGB(Mat);
Mat FloydSteinbergDithering(Mat rawImg);
Mat JJNDithering(Mat grayImg);
