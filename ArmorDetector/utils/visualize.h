#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "detector.h"

#include <opencv2/opencv.hpp>
#include <iostream>

#define CLOCKS_PER_NS 1000000000L
#define str(x)  std::to_string(x)

namespace Visualize
{
    bool enableLogInfo = true;
    bool enableGraphics = true;

    const int colorList[80][3] =
    { //{255 ,255 ,255},
        {216 , 82 , 24},
        {236 ,176 , 31},
        {125 , 46 ,141},
        {118 ,171 , 47},
        { 76 ,189 ,237},
        {238 , 19 , 46},
        { 76 , 76 , 76},
        {153 ,153 ,153},
        {255 ,  0 ,  0},
        {255 ,127 ,  0},
        {190 ,190 ,  0},
        {  0 ,255 ,  0},
        {  0 ,  0 ,255},
        {170 ,  0 ,255},
        { 84 , 84 ,  0},
        { 84 ,170 ,  0},
        { 84 ,255 ,  0},
        {170 , 84 ,  0},
        {170 ,170 ,  0},
        {170 ,255 ,  0},
        {255 , 84 ,  0},
        {255 ,170 ,  0},
        {255 ,255 ,  0},
        {  0 , 84 ,127},
        {  0 ,170 ,127},
        {  0 ,255 ,127},
        { 84 ,  0 ,127},
        { 84 , 84 ,127},
        { 84 ,170 ,127},
        { 84 ,255 ,127},
        {170 ,  0 ,127},
        {170 , 84 ,127},
        {170 ,170 ,127},
        {170 ,255 ,127},
        {255 ,  0 ,127},
        {255 , 84 ,127},
        {255 ,170 ,127},
        {255 ,255 ,127},
        {  0 , 84 ,255},
        {  0 ,170 ,255},
        {  0 ,255 ,255},
        { 84 ,  0 ,255},
        { 84 , 84 ,255},
        { 84 ,170 ,255},
        { 84 ,255 ,255},
        {170 ,  0 ,255},
        {170 , 84 ,255},
        {170 ,170 ,255},
        {170 ,255 ,255},
        {255 ,  0 ,255},
        {255 , 84 ,255},
        {255 ,170 ,255},
        { 42 ,  0 ,  0},
        { 84 ,  0 ,  0},
        {127 ,  0 ,  0},
        {170 ,  0 ,  0},
        {212 ,  0 ,  0},
        {255 ,  0 ,  0},
        {  0 , 42 ,  0},
        {  0 , 84 ,  0},
        {  0 ,127 ,  0},
        {  0 ,170 ,  0},
        {  0 ,212 ,  0},
        {  0 ,255 ,  0},
        {  0 ,  0 , 42},
        {  0 ,  0 , 84},
        {  0 ,  0 ,127},
        {  0 ,  0 ,170},
        {  0 ,  0 ,212},
        {  0 ,  0 ,255},
        {  0 ,  0 ,  0},
        { 36 , 36 , 36},
        { 72 , 72 , 72},
        {109 ,109 ,109},
        {145 ,145 ,145},
        {182 ,182 ,182},
        {218 ,218 ,218},
        {  0 ,113 ,188},
        { 80 ,182 ,188},
        {127 ,127 ,  0},
    };

    std::vector<std::string> labels = { CLASS_NAMES };

    void logInfo(Object object)
    {
        using std::cerr;
        using std::setw;
        using std::setprecision;

        cerr.setf(std::ios::fixed);
        cerr << std::left << setprecision(4);
        cerr << "LB: " << setw(8) << labels[object.label] << "Score: " << object.score;
        cerr << std::right;
        cerr << "  Center: (" << setw(4) << object.centerX << ", " << setw(4) << object.centerY << ")";
        cerr << "  w: " << setw(3) << object.width << ",  h: " << setw(3) << object.height << "\n";
    }

    void drawBoxes(cv::Mat & image, const DetectResult & result)
    {
        if (enableLogInfo) {
            std::cerr << "Image Width: " << result.imageWidth << "  Image Height: " << result.imageHeight << std::endl;
            std::cerr << "Total Objects: " << result.size() << std::endl;
        }

        for (const auto & object : result.objects) {
            cv::Scalar color(colorList[object.label][0], colorList[object.label][1], colorList[object.label][2]);
            if (enableLogInfo) logInfo(object);
            cv::Rect objectRect = object.toRect();
            cv::rectangle(image, objectRect, color, 2);
            string text = labels[object.label] + " " + str(int(object.score * 100.f)) + "%";
            cv::Point textPosition(objectRect.x, objectRect.y > 5 ? objectRect.y - 5 : 0);
            cv::putText(image, text, textPosition, cv::FONT_HERSHEY_SIMPLEX, 0.4, color);
        }
    }

    void detectVideo(const std::string & modelPath, const std::string & videoPath)
    {
        Detector detector(modelPath);
        cv::VideoCapture capture(videoPath);
        cv::Mat image;
        int frameCounter = 0;
        struct timespec startTime {0, 0};
        clock_gettime(CLOCK_REALTIME, &startTime);
        struct timespec currentTime = startTime;

        while (++frameCounter) {
            capture >> image;
            if (image.empty()) return;
            auto result = detector.detect(image);
            drawBoxes(image, result);
            std::cerr << std::endl;

            auto lastNSec = currentTime.tv_nsec;
            clock_gettime(CLOCK_REALTIME, &currentTime);
            auto FPS    = CLOCKS_PER_NS / (currentTime.tv_nsec - lastNSec);

            string frameInfo = "Frame: "  + str(frameCounter) + "  FPS: "    + str(FPS);
            std::cerr << frameInfo << std::endl;

            if (enableGraphics) {
                cv::imshow("image", image);
                cv::waitKey(1);
            }
        }
    }

    void detectWebcam(const std::string & modelPath, int webcamId)
    {
        Detector detector(modelPath);
        cv::VideoCapture capture(webcamId);
        cv::Mat image;
        int frameCounter = 0;
        struct timespec startTime {0, 0};
        clock_gettime(CLOCK_REALTIME, &startTime);
        struct timespec currentTime = startTime;

        while (++frameCounter) {
            capture >> image;
            if (image.empty()) return;
            DetectResult result = detector.detect(image);
            drawBoxes(image, result);
            std::cerr << std::endl;

            auto lastNSec = currentTime.tv_nsec;
            clock_gettime(CLOCK_REALTIME, &currentTime);
            auto FPS    = CLOCKS_PER_NS / (currentTime.tv_nsec - lastNSec);

            string frameInfo = "Frame: "  + str(frameCounter) + "  FPS: "    + str(FPS);
            std::cerr << frameInfo << std::endl;

            if (enableGraphics) {
                cv::imshow("image", image);
                cv::waitKey(1);
            }
        }
    }
}

#endif // VISUALIZE_H
