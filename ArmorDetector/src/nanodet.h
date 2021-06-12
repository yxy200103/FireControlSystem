#ifndef NANODET_H
#define NANODET_H

#include <opencv2/core/core.hpp>
#include <ncnn/net.h>
#include <ncnn/benchmark.h>

#include <iostream>
#include <string>

using std::string;

struct HeadInfo
{
    std::string cls_layer;
    std::string dis_layer;
    int stride;
};

struct BoxInfo
{
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
};

class NanoDet
{
private:

    static void preprocess(cv::Mat&, ncnn::Mat&);
    void decodeInfer(ncnn::Mat&, ncnn::Mat&, int, float, std::vector<std::vector<BoxInfo>>&);
    BoxInfo disPred2Bbox(const float*&, int, float, int, int, int) const;
    static void nmsFilter(std::vector<BoxInfo> &inputBoxes, float nmsThreshold);
    int inputSize = 0;
    int numClass  = 0;
    int regMax    = 7;
    ncnn::Net* neuralNetwork = nullptr;

public:

    static bool gpuAvailable;

    std::vector<HeadInfo> headsInfo {
        // cls_pred | dis_pred | stride
        {"792", "795",    8},
        {"814", "817",   16},
        {"836", "839",   32},
    };

    std::vector<BoxInfo> detect(cv::Mat, float, float);
    std::vector<string> labels;

    void loadModel(const char*, const char*, int, bool);
    void loadModel(const string&, const string&, int, bool);
    void setLabels(std::vector<string>);

    NanoDet(const char*, const char*, int, bool);
    NanoDet(const string&, const string&, int, bool);

    NanoDet() = default;
    ~NanoDet();
};

#endif // NANODET_H