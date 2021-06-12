#include "library.h"

Detector detector(MODEL_PATH);
cv::VideoCapture capture(CAPTURE_SOURCE);

extern "C"
{
    EXPORT void detect(float & x, float & y)
    {
        cv::Mat image;
        capture >> image;
        auto result = detector.detect(image);
        Object* best = Detector::getBest(result);

        if (best != nullptr) {
            x = (best->centerX / (float) result.imageWidth)  * 2 - 1;
            y = (best->centerY / (float) result.imageHeight) * 2 - 1;
            delete best;
        }
    }
};


