
#ifndef ENERGY_GETRECT_H
#define ENERGY_GETRECT_H

#include <iostream>
#include <opencv2/opencv.hpp>

using cv::Mat;

struct target {
    int x, y;
};

class Detect{

public:

    Detect detect(const Mat& midImage, Mat srcImage);

    static double getDistance(cv::Point2f point1, cv::Point2f point2);

    static double TemplateMatch(cv::Mat image, cv::Mat tepl, cv::Point &point, int method);

    static bool CircleInfo2(std::vector <cv::Point2f> &pts, cv::Point2f &center, float &radius);

    cv::Point2f GetCircleCenter(cv::RotatedRect targetFan, cv::Point2f centerP);

    int radius;
    cv::Point2f circleCenter;
    cv::Point2f targetArmor;
};


#endif //ENERGY_GETRECT_H
