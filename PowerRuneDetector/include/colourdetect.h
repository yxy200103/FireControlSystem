#ifndef RBDETECT_H
#define RBDETECT_H

#include <opencv2/opencv.hpp>

using cv::Mat;

class ColourDetect {

public:

    int detect(Mat srcImage);
};


#endif //RBDETECT_H
