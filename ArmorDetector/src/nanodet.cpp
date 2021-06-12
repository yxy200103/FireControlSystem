#include "nanodet.h"
#include "config.h"

#include <opencv2/core/core.hpp>
#include <ncnn/net.h>
#include <ncnn/benchmark.h>

#include <iostream>
#include <string>

using std::string;

inline float fastExp(float x)
{
    union {
        uint32_t uint32;
        float    float32;
    } value { };

    value.uint32 = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return value.float32;
}

inline float sigmoid(float x)
{
    return 1.0f / (1.0f + fastExp(-x));
}

template<typename Tp>
int activationFunctionSoftmax(const Tp* src, Tp* destination, int length)
{
    const Tp alpha = *std::max_element(src, src + length);
    Tp denominator { 0 };

    for (int i = 0; i < length; ++i) {
        destination[i] = fastExp(src[i] - alpha);
        denominator += destination[i];
    }

    for (int i = 0; i < length; ++i) {
        destination[i] /= denominator;
    }

    return 0;
}

bool NanoDet::gpuAvailable = true;

void NanoDet::loadModel(const char* param, const char* bin, int size, bool useGPU = false)
{
    this->neuralNetwork = new ncnn::Net();
    NanoDet::gpuAvailable = ncnn::get_gpu_count() > 0;
    this->neuralNetwork->opt.use_vulkan_compute = NanoDet::gpuAvailable && useGPU;
    this->neuralNetwork->opt.use_fp16_arithmetic = true;
    this->neuralNetwork->load_param(param);
    this->neuralNetwork->load_model(bin);
    this->inputSize = size;
}

void NanoDet::loadModel(const std::string & param, const std::string & bin, int size, bool useGPU = false)
{
    loadModel(param.c_str(), bin.c_str(), size, useGPU);
}

void NanoDet::setLabels(std::vector<string> labels)
{
    this->labels = labels;
    this->numClass = labels.size();
}

NanoDet::NanoDet(const char* param, const char* bin, int size, bool useGPU = false)
{
    loadModel(param, bin, size, useGPU);
}

NanoDet::NanoDet(const std::string & param, const std::string & bin, int size, bool useGPU = false)
{
    loadModel(param, bin, size, useGPU);
}

NanoDet::~NanoDet() = default;

void NanoDet::preprocess(cv::Mat & image, ncnn::Mat & input)
{
    int imageWidth  = image.cols;
    int imageHeight = image.rows;

    input = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR, imageWidth, imageHeight);

    const float meanValues[3] = { 103.53f, 116.28f, 123.675f };
    const float normValues[3] = { 0.017429f, 0.017507f, 0.017125f };
    input.substract_mean_normalize(meanValues, normValues);
}

std::vector<BoxInfo> NanoDet::detect(cv::Mat image, float scoreThreshold, float nmsThreshold)
{
    ncnn::Mat input;
    preprocess(image, input);

    auto extractor = this->neuralNetwork->create_extractor();
    extractor.set_light_mode(false);
    extractor.set_num_threads(4);
    extractor.input("input.1", input); // name in .param file

    std::vector<std::vector<BoxInfo>> results;
    results.resize(this->numClass);

    for (const auto& headInfo : this->headsInfo) {
        ncnn::Mat disPredict;
        ncnn::Mat clsPredict;
        extractor.extract(headInfo.dis_layer.c_str(), disPredict);
        extractor.extract(headInfo.cls_layer.c_str(), clsPredict);
        this->decodeInfer(clsPredict, disPredict, headInfo.stride, scoreThreshold, results);
    }

    std::vector<BoxInfo> dets;
    for (auto & result : results) {
        NanoDet::nmsFilter(result, nmsThreshold);
        for (auto box : result) {
            dets.push_back(box);
        }
    }

    return dets;
}

void NanoDet::decodeInfer(ncnn::Mat& classPredict, ncnn::Mat& disPredict, int stride, float threshold,
                          std::vector<std::vector<BoxInfo>>& results)
{
    int feature_h = this->inputSize / stride;
    int feature_w = this->inputSize / stride;

    for (int idx = 0; idx < feature_h * feature_w; idx++) {
        const float* scores = classPredict.row(idx);
        int row = idx / feature_w;
        int col = idx % feature_w;
        float score = 0;
        int cur_label = 0;

        for (int label = 0; label < this->numClass; label++) {
            if (scores[label] > score) {
                score = scores[label];
                cur_label = label;
            }
        }

        if (score > threshold) {
            const float* bbox_pred = disPredict.row(idx);
            results[cur_label].push_back(this->disPred2Bbox(bbox_pred, cur_label, score, col, row, stride));
        }
    }
}

BoxInfo NanoDet::disPred2Bbox(const float* & dfl_det, int label, float score, int x, int y, int stride) const
{
    float ct_x = (x + 0.5f) * stride;
    float ct_y = (y + 0.5f) * stride;
    std::vector<float> dis_pred;
    dis_pred.resize(4);
    for (int i = 0; i < 4; i++) {
        float dis = 0;
        auto* dis_after_sm = new float[this->regMax + 1];

        activationFunctionSoftmax(dfl_det + i * (this->regMax + 1), dis_after_sm, this->regMax + 1);
        for (int j = 0; j < this->regMax + 1; j++) {
            dis += j * dis_after_sm[j];
        }

        dis *= stride;
        dis_pred[i] = dis;
        delete[] dis_after_sm;
    }
    float xmin = (std::max)(ct_x - dis_pred[0], .0f);
    float ymin = (std::max)(ct_y - dis_pred[1], .0f);
    float xmax = (std::min)(ct_x + dis_pred[2], (float)this->inputSize);
    float ymax = (std::min)(ct_y + dis_pred[3], (float)this->inputSize);

    return BoxInfo { xmin, ymin, xmax, ymax, score, label };
}

void NanoDet::nmsFilter(std::vector<BoxInfo> & inputBoxes, float nmsThreshold)
{
    std::sort(inputBoxes.begin(), inputBoxes.end(), [](BoxInfo a, BoxInfo b) {
        return a.score > b.score;
    });

    std::vector<float> vArea(inputBoxes.size());
    for (int i = 0; i < inputBoxes.size(); ++i) {
        vArea[i] = (inputBoxes[i].x2 - inputBoxes[i].x1 + 1) * (inputBoxes[i].y2 - inputBoxes[i].y1 + 1);
    }

    for (int i = 0; i < inputBoxes.size(); i++) {
        for (int j = i + 1; j < inputBoxes.size();) {
            float xx1 = std::max(inputBoxes[i].x1, inputBoxes[j].x1);
            float yy1 = std::max(inputBoxes[i].y1, inputBoxes[j].y1);
            float xx2 = std::min(inputBoxes[i].x2, inputBoxes[j].x2);
            float yy2 = std::min(inputBoxes[i].y2, inputBoxes[j].y2);
            float w   = std::max(float(0), xx2 - xx1 + 1);
            float h   = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);

            if (ovr >= nmsThreshold) {
                inputBoxes.erase(inputBoxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            } else {
                j += 1;
            }
        }
    }
}
