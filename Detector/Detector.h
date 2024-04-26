#pragma once

#ifndef RM2024_DETECTOR_H
#define RM2024_DETECTOR_H


#include <opencv2/core.hpp>
#include "opencv2/highgui.hpp"

#include "opencv2/imgproc.hpp"
// STL
#include <algorithm>
#include <string>
#include "box.h"


namespace ImgProcess {


    class Detector
    {
    public:
        struct LightParams
        {
            double min_ratio; // 宽高比的最小值
            double max_ratio; // 宽高比的最大值
            double min_area; // 宽高比的最小值
            double max_area; // 宽高比的最大值
        };

        // 定义一个结构体ArmorParams，用于存储关于装甲板的参数
        struct ArmorParams
        {
            double min_pair_ratio;           // 光源比例的最小值
            double min_center_distance;
            double max_center_distance;
        };

        // Detector类的构造函数，初始化检测参数
        Detector(const int &bin_thres, const LightParams &l, const ArmorParams &a);

        // 检测输入图像中的装甲板，并返回一个装甲板对象的向量
        std::vector<InsideBox> detect(const cv::Mat &input);

        // 对输入图像进行预处理，并返回预处理后的图像
        cv::Mat preprocessImage(const cv::Mat &input);

        // 在RBG图像和二值图像中找到光源，并返回一个光源对象的向量
        std::vector<Pair> findPairs(const cv::Mat &binary_img);

        // 匹配找到的光源，以确定可能的装甲板，并返回一个装甲板对象的向量
        std::vector<InsideBox> matchLights(std::vector<Pair> &lights);

        // 二值化阈值
        int binary_thres=220;

        // 在给定的图像上绘制检测结果
        void drawResults(cv::Mat &img);

        // 光源参数
        LightParams l;
        // 装甲板参数
        ArmorParams a;

        cv::Mat binary_img;
    private:
        // 判断给定的对象是否为光源
        bool isPair(const Pair &possible_light);

        // 判断两个光源之间是否存在其他光源
        bool containLight(
                const Pair &light_1, const Pair &light_2, const std::vector<Pair> &lights);

        // 根据两个光源判断是否构成装甲板，并返回装甲板类型
        bool isInsideBox(const Pair &light_1, const Pair &light_2);

        // 存储找到的光源
        std::vector<Pair> lights_;

        // 存储匹配成功的装甲板
        std::vector<InsideBox> armors_;
    };

}
#endif //RM2024_DETECTOR_H
