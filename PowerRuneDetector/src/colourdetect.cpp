#include "colourdetect.h"
#include "config.h"

#include <opencv2/opencv.hpp>
#include <vector>

using cv::Mat;

int ColourDetect::detect(Mat srcImage) {
    std::vector<Mat> imgChannels;
    split(srcImage, imgChannels);

    Mat midImageBlue = imgChannels.at(0) - imgChannels.at(2);
    Mat midImageRed  = imgChannels.at(2) - imgChannels.at(0);

    //二值化
    threshold(midImageBlue, midImageBlue, 100, 255, cv::THRESH_BINARY);
    threshold(midImageRed, midImageRed, 100, 255, cv::THRESH_BINARY);

    int redArea, blueArea;
    int height,width;

    //Height and width are same in two images
    height = midImageBlue.rows;
    width = midImageBlue.cols;

    for(int row = 0; row < height; row++) {
        const uchar * ptr = midImageBlue.ptr(row);
        for(int col = 0; col < width; col++) {
            blueArea += ptr[0];
            ptr++;
        }
    }

    for(int row = 0; row < height; row++) {
        const uchar * ptr = midImageBlue.ptr(row);
        for(int col = 0; col < width; col++) {
            redArea += ptr[0];
            ptr++;
        }
    }

    if(blueArea > redArea){
        return BLUE;
    }
    else{
        return RED;
    }
}