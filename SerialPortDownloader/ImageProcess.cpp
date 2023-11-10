#include "ImageProcess.h"


QString image_to_str(Mat image)
{
    stringstream ss;
    for (int i = 0; i < image.rows; i++) {
        string temp_str = "";
        for (int j = 0; j < image.cols; j++) {
            temp_str += image.at<uchar>(i, j) ? "1" : "0";
            if ((j + 1) % 4 == 0) {
                ss << hex << stoi(temp_str, nullptr, 2);
                temp_str.clear(); // 清空temp_str
            }
        }
    }
    return QString::fromStdString(ss.str());
}


Mat convertToRGB(Mat image)
{
    Mat converted;
    switch (image.channels()) {
    case 1: // 灰度图
        cvtColor(image, converted, COLOR_GRAY2RGB);
        break;
    case 3: // BGR 彩色图
        cvtColor(image, converted, COLOR_BGR2RGB);
        break;
    case 4: // BGRA 彩色图
        cvtColor(image, converted, COLOR_BGRA2RGB);
        break;
    default: // 其他情况，可能是二值化图像
        cvtColor(image, converted, COLOR_GRAY2RGB);
        break;
    }
    return converted;
}


uint8_t saturated_add(uint8_t val1, int8_t val2)
{
    int16_t tmp = static_cast<int16_t>(val1) + static_cast<int16_t>(val2);

    if (tmp > 255)
    {
        return 255;
    }
    else if (tmp < 0)
    {
        return 0;
    }
    else
    {
        return static_cast<uint8_t>(tmp);
    }
}


// Floyd–Steinberg dithering only diffuses the error to neighboring pixels. This results in very fine-grained dithering.
Mat FloydSteinbergDithering(Mat grayImg) {
    // Grayscale First
    Mat dithImg = grayImg;

    // Run the 'Floyd-Steinberg' dithering algorithm ...
    int imgWidth = dithImg.cols;
    int imgHeight = dithImg.rows;
    uint8_t* rowPtr = dithImg.ptr<uint8_t>();

    for (int i = 0; i < imgHeight; i++)
    {
        uint8_t* pixelPtr = rowPtr;

        for (int j = 0; j < imgWidth; j++)
        {
            int err, a, b, c, d;

            if (*pixelPtr > 127)
            {
                err = *pixelPtr - 255;
                *pixelPtr = 255;
            }
            else
            {
                err = *pixelPtr - 0;
                *pixelPtr = 0;
            }

            a = (err * 7) / 16;
            b = (err * 1) / 16;
            c = (err * 5) / 16;
            d = (err * 3) / 16;

            if (i != (imgHeight - 1) && j != 0 && j != (imgWidth - 1))
            {
                *(pixelPtr + 1) = saturated_add(*(pixelPtr + 1), a);
                *(pixelPtr + imgWidth + 1) = saturated_add(*(pixelPtr + imgWidth + 1), b);
                *(pixelPtr + imgWidth) = saturated_add(*(pixelPtr + imgWidth), c);
                *(pixelPtr + imgWidth - 1) = saturated_add(*(pixelPtr + imgWidth - 1), d);
            }
            pixelPtr++;
        }
        rowPtr += imgWidth;
    }
    return dithImg;
}


// Minimized average error dithering by Jarvis, Judice, and Ninke diffuses the error also to pixels one step further away. 
Mat JJNDithering(Mat grayImg) {
    Mat dithImg;

    // Convert grayscale image to float type for calculation
    Mat imgFloat;
    grayImg.convertTo(imgFloat, CV_32FC1);

    int imgWidth = imgFloat.cols;
    int imgHeight = imgFloat.rows;
    cv::Rect roi(1, 1, imgWidth - 2, imgHeight - 2);
    for (int i = 0; i < imgHeight; i++)
    {
        for (int j = 0; j < imgWidth; j++)
        {
            float err, a, b, c, d, e, f, g, h, k;

            if (imgFloat.at<float>(i, j) > 127)
            {
                err = imgFloat.at<float>(i, j) - 255.0f;
                imgFloat.at<float>(i, j) = 255.0f;
            }
            else
            {
                err = imgFloat.at<float>(i, j) - 0.0f;
                imgFloat.at<float>(i, j) = 0.0f;
            }

            a = (err * 7.0f) / 48.0f;
            b = (err * 5.0f) / 48.0f;
            c = (err * 3.0f) / 48.0f;
            d = (err * 5.0f) / 48.0f;
            e = (err * 7.0f) / 48.0f;
            f = (err * 5.0f) / 48.0f;
            g = (err * 3.0f) / 48.0f;
            h = (err * 5.0f) / 48.0f;
            k = (err * 7.0f) / 48.0f;

            if (roi.contains(cv::Point(j + 1, i))) // right
                imgFloat.at<float>(i, j + 1) = saturated_add(imgFloat.at<float>(i, j + 1), a);
            if (roi.contains(cv::Point(j + 2, i))) // right-right
                imgFloat.at<float>(i, j + 2) = saturated_add(imgFloat.at<float>(i, j + 2), b);
            if (roi.contains(cv::Point(j - 2, i + 1))) // bottom-left
                imgFloat.at<float>(i + 1, j - 2) = saturated_add(imgFloat.at<float>(i + 1, j - 2), c);
            if (roi.contains(cv::Point(j - 1, i + 1))) // bottom
                imgFloat.at<float>(i + 1, j - 1) = saturated_add(imgFloat.at<float>(i + 1, j - 1), d);
            if (roi.contains(cv::Point(j, i + 1))) // bottom-center
                imgFloat.at<float>(i + 1, j) = saturated_add(imgFloat.at<float>(i + 1, j), e);
            if (roi.contains(cv::Point(j + 1, i + 1))) // bottom-right
                imgFloat.at<float>(i + 1, j + 1) = saturated_add(imgFloat.at<float>(i + 1, j + 1), f);
            if (roi.contains(cv::Point(j - 1, i + 2))) // bottom-left-left
                imgFloat.at<float>(i + 2, j - 1) = saturated_add(imgFloat.at<float>(i + 2, j - 1), g);
            if (roi.contains(cv::Point(j, i + 2))) // bottom-left
                imgFloat.at<float>(i + 2, j) = saturated_add(imgFloat.at<float>(i + 2, j), h);
            if (roi.contains(cv::Point(j + 1, i + 2))) // bottom-right
                imgFloat.at<float>(i + 2, j + 1) = saturated_add(imgFloat.at<float>(i + 2, j + 1), k);
        }
    }

    // Convert image back to uint8_t type for output
    imgFloat.convertTo(dithImg, CV_8UC1);

    return dithImg;
}
