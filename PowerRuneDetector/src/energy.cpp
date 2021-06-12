#include "threshold.h"
#include "detect.h"
#include "config.h"
#include "colourdetect.h"
#include "library.h"

#include <iostream>
#include <opencv2/opencv.hpp>

//#define fitting_60
//#define COLOURDETECT

#define DETECT_FAILED -1

using cv::Mat;

extern int targetColour;

int main() {
    cv::VideoCapture capture("red.mp4");
    std::vector<cv::Point2f> cirV;
    Mat srcImage;
    Mat drawcircle;

    for (int i = 0; i < 123; i++) {

        capture >> srcImage;
        drawcircle = srcImage;

        #ifdef COLOURDETECT
        ColourDetect colourDetect;
        #endif

        switch(targetColour){

            #ifdef COLOURDETECT
            case (HAVE_NOT_DETECTED): {
                colourDetect.detect(srcImage);
            }
            #endif

            case (RED || BLUE): {
                break;
            }
            default: {
                throw std::exception();
            }
        }

        Mat midImage;
        Threshold threshold;
        midImage = threshold.handle(srcImage, targetColour);

        Detect detect;
        detect = detect.detect(midImage, srcImage);
        if(detect.radius != DETECT_FAILED) {
            cv::circle(drawcircle, detect.circleCenter, detect.radius, cv::Scalar(255, 255, 255), 5);
            cv::circle(drawcircle, detect.targetArmor, 5, cv::Scalar(0, 0, 255), 5);
        }

        cv::imwrite("haha" + std::to_string(i) + ".jpg", drawcircle);

        #ifdef fitting_60
        target targetVector;
        cv::Point targetArmor;

        if (cirV.size() >= 3) {
            float R;
            //得到拟合的圆心
            Detect::CircleInfo2(cirV, cc, R);
            circle(drawcircle, cc, R, cv::Scalar(255, 255, 255), 2);

            cv::Point2f cd;
            cd.x = cc.x;
            cd.y = cc.y + R;
            circle(drawcircle, cd, 5, cv::Scalar(255, 150, 200), 10);

            cv::Point2f targetArmorPoint;
            targetArmorPoint.x = targetArmor.x;
            targetArmorPoint.y = targetArmor.y;
            circle(drawcircle, targetArmorPoint, 5, cv::Scalar(200, 100, 50), 10);

            cv::imwrite("haha" + std::to_string(i) + ".jpg", drawcircle);

            targetVector.x = (int)cc.x - srcImage.cols;
            targetVector.y = (int)cc.y + (int) R - srcImage.rows;
        }
        cv::imwrite("haha" + std::to_string(i) + ".jpg", output);
        #endif

    }
}