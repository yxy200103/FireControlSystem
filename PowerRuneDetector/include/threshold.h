
#ifndef ENERGY_THRESHOLD_H
#define ENERGY_THRESHOLD_H

#include "config.h"

#include <iostream>
#include <opencv2/opencv.hpp>

using cv::Mat;

class Threshold{

    public:

        Mat handle(Mat srcImage, int targetColour);

};


#endif //ENERGY_THRESHOLD_H

