/* * * * * * * * * * * * * * * * * * * * * * * * *\
|*                                               *|
|*      File     :   detector.h                  *|
|*      Author   :   UNNC RM Software Group      *|
|*      Date     :   2021/01/16                  *|
|*      Version  :   Beta 1.0                    *|
|*                                               *|
\* * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once
#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <utility>

#include "nanodet.h"
#include "config.h"

using std::string;

struct Object
{
    int centerX, centerY;
    int width, height;

    int label;
    float score;

    explicit Object(BoxInfo bbox, int imageWidth, int imageHeight, const cv::Rect & effectedArea)
    {
        this->centerX = round(imageWidth  * (bbox.x1 + bbox.x2 - 2.f * effectedArea.x) / (2.f * effectedArea.width));
        this->centerY = round(imageHeight * (bbox.y1 + bbox.y2 - 2.f * effectedArea.y) / (2.f * effectedArea.height));
        this->width   = round(imageWidth  * (bbox.x2 - bbox.x1) / effectedArea.width);
        this->height  = round(imageHeight * (bbox.y2 - bbox.y1) / effectedArea.height);
        this->label   = bbox.label;
        this->score   = bbox.score;
    }

    [[nodiscard]]
    cv::Rect toRect() const
    {
        int xmin = centerX - width  / 2;
        int ymin = centerY - height / 2;
        return cv::Rect(xmin, ymin, width, height);
    }
};

struct DetectResult
{
    int imageWidth;
    int imageHeight;

    std::vector<Object> objects;
    [[nodiscard]] size_t size() const { return objects.size(); }

    explicit DetectResult(const std::vector<BoxInfo> & boxes, int imageWidth, int imageHeight,
                          const cv::Rect & effectedArea)
    {
        this->imageWidth  = imageWidth;
        this->imageHeight = imageHeight;
        for (const BoxInfo & box : boxes) {
            objects.emplace_back(Object(box, imageWidth, imageHeight, effectedArea));
        }
    }

    Object& operator[] (int index)
    {
        return objects.at(index);
    }
};

class Detector
{
private:

    NanoDet nanodet;
    static cv::Rect getAvailableAreaSize(int width, int height);
    static cv::Rect getAvailableAreaSize(cv::Size size);
    static cv::Mat resizeUniform(cv::Mat sourceImage);

public:

    explicit Detector(const string & modelName);
    DetectResult detect(cv::Mat image);
    static Object* getBest(DetectResult & result, float threshold = 0.6);
};

#endif // DETECTOR_H
