#include "detector.h"
#include "nanodet.h"
#include "config.h"

#include <opencv2/opencv.hpp>

cv::Rect Detector::getAvailableAreaSize(int width, int height)
{
    if (width > height) {
        int resizedY = INPUT_SIZE * (width - height) / (2 * width);
        int resizedHeight = INPUT_SIZE * height / width;
        return cv::Rect_<int>(0, resizedY, INPUT_SIZE, resizedHeight);
    }
    else if (width < height) {
        int resizedX = INPUT_SIZE * (height - width) / (2 * height);
        int resizedWidth = INPUT_SIZE * width / height;
        return cv::Rect_<int>(resizedX, 0, resizedWidth, INPUT_SIZE);
    }
    else /* width == height */ {
        return cv::Rect_<int>(0, 0, INPUT_SIZE, INPUT_SIZE);
    }
}

cv::Rect Detector::getAvailableAreaSize(cv::Size size)
{
    return getAvailableAreaSize(size.width, size.height);
}

cv::Mat Detector::resizeUniform(cv::Mat sourceImage)
{
    cv::Mat resizedImage;
    cv::Rect effectedArea = getAvailableAreaSize(sourceImage.cols, sourceImage.rows);

    if (effectedArea.width == effectedArea.height) {
        cv::resize(sourceImage, resizedImage, cv::Size(INPUT_SIZE, INPUT_SIZE));
        return resizedImage;
    }

    resizedImage = cv::Mat(cv::Size(INPUT_SIZE, INPUT_SIZE), CV_8UC3, cv::Scalar(0));
    cv::resize(sourceImage, sourceImage, cv::Size(effectedArea.width, effectedArea.height));
    cv::Mat roiImage = resizedImage(effectedArea);
    sourceImage.copyTo(roiImage);

    return resizedImage;
}

Detector::Detector(const string & modelName)
{
    nanodet.loadModel(modelName + ".param", modelName + ".bin", INPUT_SIZE, USE_GPU);
    nanodet.setLabels({ CLASS_NAMES });
}

DetectResult Detector::detect(cv::Mat image)
{
    auto imageSize = image.size();
    std::vector<BoxInfo> boxes = nanodet.detect(resizeUniform(std::move(image)), SCORE_THRESHOLD, NMS_THRESHOLD);
    return DetectResult(boxes, imageSize.width, imageSize.height, getAvailableAreaSize(imageSize));
}

Object* Detector::getBest(DetectResult &result, float threshold)
{
    Object* bestObject = nullptr;
    for (int i = 0; i < result.size(); i++) {
        if (!bestObject || result[i].height >= bestObject->height && result[i].score > threshold) {
            if (result[i].width >= result[i].height * 1.2) {
                bestObject = &result[i];
            }
        }
    }

    return bestObject;
}
