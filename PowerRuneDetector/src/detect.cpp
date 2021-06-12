
#include "detect.h"
#include "config.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>

//#define CIRCLE_FILLING


using cv::Mat;
using std::vector;

double Detect::getDistance(cv::Point2f point1, cv::Point2f point2) {

    double width, hight, dis;
    width = abs(point1.x - point2.x);
    hight = abs(point1.y - point2.y);
    dis = sqrt(pow(width, 2) + pow(hight, 2));
    return dis;

}

double Detect::TemplateMatch(cv::Mat image, cv::Mat tepl, cv::Point &point, int method) {
    int result_cols = image.cols - tepl.cols + 1;
    int result_rows = image.rows - tepl.rows + 1;

    cv::Mat result = cv::Mat(result_cols, result_rows, CV_32FC1);
    cv::matchTemplate(image, tepl, result, method);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    switch (method) {
        case cv::TM_SQDIFF:
        case cv::TM_SQDIFF_NORMED:
            point = minLoc;
            return minVal;

        default:
            point = maxLoc;
            return maxVal;

    }
}

cv::Point2f Detect::GetCircleCenter(cv::RotatedRect targetFan, cv::Point2f centerP) {
    cv::Point2f fanPoint[4];
    targetFan.points(fanPoint);

    cv::Point2f tmp;

    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            // Manhattan distance
            if (abs(centerP.x - fanPoint[i].x) + abs(centerP.y - fanPoint[i].y) <
                abs(centerP.x - fanPoint[j].x) + abs(centerP.y - fanPoint[j].y)) {
                tmp = fanPoint[j];
                fanPoint[j] = fanPoint[i];
                fanPoint[i] = tmp;
            }
        }
    }


    cv::Point2f midPoint;
    midPoint.x = (fanPoint[0].x + fanPoint[1].x) / 2;
    midPoint.y = (fanPoint[0].y + fanPoint[1].y) / 2;

    double vectorX, vectorY;
    vectorX = centerP.x - midPoint.x;
    vectorY = centerP.y - midPoint.y;

    const double ccconst = -0.33;
    cv::Point2f circleCenter;
    circleCenter.x = vectorX * ccconst + midPoint.x;
    circleCenter.y = vectorY * ccconst + midPoint.y;

    return circleCenter;
}

#ifdef CIRCLE_FILLING
bool Detect::CircleInfo2(std::vector<cv::Point2f> &pts, cv::Point2f &center, float &radius) {
    center = cv::Point2d(0, 0);
    radius = 0.0;
    if (pts.size() < 3) {
        return false;
    }

    double sumX = 0.0;
    double sumY = 0.0;
    double sumX2 = 0.0;
    double sumY2 = 0.0;
    double sumX3 = 0.0;
    double sumY3 = 0.0;
    double sumXY = 0.0;
    double sumX1Y2 = 0.0;
    double sumX2Y1 = 0.0;
    const double N = (double) pts.size();
    for (int i = 0; i < pts.size(); ++i) {
        double x = pts.at(i).x;
        double y = pts.at(i).y;
        double x2 = x * x;
        double y2 = y * y;
        double x3 = x2 * x;
        double y3 = y2 * y;
        double xy = x * y;
        double x1y2 = x * y2;
        double x2y1 = x2 * y;

        sumX += x;
        sumY += y;
        sumX2 += x2;
        sumY2 += y2;
        sumX3 += x3;
        sumY3 += y3;
        sumXY += xy;
        sumX1Y2 += x1y2;
        sumX2Y1 += x2y1;
    }
    double C = N * sumX2 - sumX * sumX;
    double D = N * sumXY - sumX * sumY;
    double E = N * sumX3 + N * sumX1Y2 - (sumX2 + sumY2) * sumX;
    double G = N * sumY2 - sumY * sumY;
    double H = N * sumX2Y1 + N * sumY3 - (sumX2 + sumY2) * sumY;

    double denominator = C * G - D * D;
    if (std::abs(denominator) < DBL_EPSILON) return false;
    double a = (H * D - E * G) / (denominator);
    denominator = D * D - G * C;
    if (std::abs(denominator) < DBL_EPSILON) return false;
    double b = (H * C - E * D) / (denominator);
    double c = -(a * sumX + b * sumY + sumX2 + sumY2) / N;

    center.x = a / (-2);
    center.y = b / (-2);
    radius = std::sqrt(a * a + b * b - 4 * c) / 2;
    return true;
}
#endif

Detect Detect::detect(const Mat &midImage, Mat srcImage) {

    Mat detected;
    Mat drawcircle = midImage;
    cv::Point2f cc = cv::Point2f(0, 0);

    //查找轮廓
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    findContours(midImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    cv::RotatedRect rect_tmp2;
    bool findTarget = 0;

    Mat templ[9];
    for (int i = 1; i <= 8; i++) {
        templ[i] = imread("template/template" + std::to_string(i) + ".jpg", cv::IMREAD_GRAYSCALE);
    }

    //遍历轮廓
    if (hierarchy.empty()) {
        Detect noTarget;
        noTarget.radius = -1;
        return noTarget;
    }
    //The first element in hierarchy[] must be the outermost outline
    for (int i = 0; i >= 0; i = hierarchy[i][0]) {
        rect_tmp2 = minAreaRect(contours[i]);
        cv::Point2f P[4];
        rect_tmp2.points(P);

        cv::Point2f srcRect[4];
        cv::Point2f dstRect[4];

        double width;
        double height;

        //矫正提取的叶片的宽高
        width = getDistance(P[0], P[1]);
        height = getDistance(P[1], P[2]);
        if (width > height) {
            srcRect[0] = P[0];
            srcRect[1] = P[1];
            srcRect[2] = P[2];
            srcRect[3] = P[3];
        } else {
            cv::swap(width, height);
            srcRect[0] = P[1];
            srcRect[1] = P[2];
            srcRect[2] = P[3];
            srcRect[3] = P[0];
        }

        double area = height * width;
        if (area > 5000) {
            dstRect[0] = cv::Point2f(0, 0);
            dstRect[1] = cv::Point2f(width, 0);
            dstRect[2] = cv::Point2f(width, height);
            dstRect[3] = cv::Point2f(0, height);
            // 应用透视变换，矫正成规则矩形
            Mat transform = getPerspectiveTransform(srcRect, dstRect);
            Mat perspectMat;
            warpPerspective(midImage, perspectMat, transform, midImage.size());

//                cv::imwrite("hhh.jpg", midImage);

            // 提取扇叶图片
            Mat testim;
            testim = perspectMat(cv::Rect(0, 0, width, height));

            cv::Point matchLoc;
            double value;
            Mat tmp1;

            resize(testim, tmp1, cv::Size(42, 20));

            vector<double> Vvalue1;
            vector<double> Vvalue2;
            for (int j = 1; j <= 6; j++) {
                value = TemplateMatch(tmp1, templ[j], matchLoc, cv::TM_CCOEFF_NORMED);
                Vvalue1.push_back(value);
            }
            for (int j = 7; j <= 8; j++) {
                value = TemplateMatch(tmp1, templ[j], matchLoc, cv::TM_CCOEFF_NORMED);
                Vvalue2.push_back(value);
            }
            int maxv1 = 0, maxv2 = 0;

            for (int t1 = 0; t1 < 6; t1++) {
                if (Vvalue1[t1] > Vvalue1[maxv1]) {
                    maxv1 = t1;
                }
            }
            for (int t2 = 0; t2 < 2; t2++) {
                if (Vvalue2[t2] > Vvalue2[maxv2]) {
                    maxv2 = t2;
                }
            }

            //预测是否是要打击的扇叶
            if (Vvalue1[maxv1] > Vvalue2[maxv2] && Vvalue1[maxv1] > 0.6) {
                findTarget = true;
                //查找装甲板
                if (hierarchy[i][2] >= 0) {
                    cv::RotatedRect rect_tmp = minAreaRect(contours[hierarchy[i][2]]);
                    cv::RotatedRect targetFan = minAreaRect(contours[i]);

                    cv::Point2f Pnt[4];
                    rect_tmp.points(Pnt);

                    float width = rect_tmp.size.width;
                    float height = rect_tmp.size.height;
                    if (height > width)
                        cv::swap(height, width);
                    float area = width * height;

                    if (height / width > MAX_HEIGHT_WIDTH_RATIO || area > MAX_AREA || area < MIN_AREA) {
                        continue;
                    }
                    cv::Point centerP = rect_tmp.center;

                    cv::Point2f circleCenter;

                    circleCenter = GetCircleCenter(targetFan, centerP);

                    int radius;
                    radius = (int) sqrt(pow(centerP.x - circleCenter.x, 2) +
                                        pow(centerP.y - circleCenter.y, 2));

                    Detect detected;
                    detected.radius = radius;
                    detected.circleCenter = circleCenter;
                    detected.targetArmor = centerP;

                    return detected;

                }
            }
        }
    }
}

//cv::imwrite("haha" + std::to_string(i) + ".jpg", testim);