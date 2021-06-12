#include "threshold.h"
#include "config.h"

#include <vector>


Mat Threshold::handle(Mat srcImage, int targetColour) {
    //分割颜色通道
    std::vector<cv::Mat> imgChannels;

    split(srcImage, imgChannels);

    cv::Mat midImage;
    //获得目标颜色图像的二值图
    switch (targetColour) {
        //Target is blue
        case BLUE: {
            midImage = imgChannels.at(0) - imgChannels.at(2);
            break;
        }
        //Target is red
        case RED: {
            midImage = imgChannels.at(2) - imgChannels.at(0);
            break;
        }
        default: {
            throw std::exception();
        }
    }


    //二值化，背景为黑色，图案为白色
    //用于查找扇叶
    threshold(midImage, midImage, 100, 255, cv::THRESH_BINARY);

    int structElementSize = 2;
    cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                            cv::Size(2 * structElementSize + 1, 2 * structElementSize + 1),
                                            cv::Point(structElementSize, structElementSize));


    //膨胀
    dilate(midImage, midImage, element);
    //开运算，消除扇叶上可能存在的小洞
    structElementSize = 3;
    element = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * structElementSize + 1, 2 * structElementSize + 1),
                                    cv::Point(structElementSize, structElementSize));
    morphologyEx(midImage, midImage, cv::MORPH_CLOSE, element);

//    cv::imwrite("hah.jpg", midImage);

    return midImage;

}
