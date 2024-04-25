#pragma once

#ifndef RM2024_DETECTOR_H
#define RM2024_DETECTOR_H


#include <opencv2/core.hpp>

// STL
#include <algorithm>
#include <string>


namespace ImgProcess {
    // 定义代表红色和蓝色的常量
    const int LEFT = 0;
    const int RIGHT = 1;
    struct Pair : public cv::RotatedRect
    {
        // 默认构造函数
        Pair() = default;

        explicit Pair(cv::RotatedRect box) : cv::RotatedRect(box)
        {
            cv::Point2f p[4];
            box.points(p); // 获取旋转矩形的四个顶点
            // 按照y坐标对顶点进行排序，以确定上下顶点
            std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
            { return a.x < b.x; });
            // 计算上下中心点
            left = (p[0] + p[1]) / 2;
            right = (p[2] + p[3]) / 2;
            std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
            { return a.y < b.y; });
            top = (p[0] + p[1]) / 2;
            bottom = (p[2] + p[3]) / 2;
            // 计算光源的长度和宽度
            width = cv::norm(left - right);
            length = cv::norm(p[0] - p[1]);
            // 计算光源的倾斜角度
            tilt_angle = std::atan2(std::abs(left.x - right.x), std::abs(left.y - right.y));
            tilt_angle = tilt_angle / CV_PI * 180; // 转换为度
        }

        int type;
        // 光源的上下中心点
        cv::Point2f left, right,top,bottom;
        // 光源的长度和宽度
        double length;
        double width;
        // 光源的倾斜角度
        double tilt_angle;
    };
    struct InsideBox
    {
        // 默认构造函数
        InsideBox() = default;
        // 构造函数，接受两个光源作为参数
        InsideBox(const Pair &l1, const Pair &l2)
        {
            // 确定左右光源
            if (l1.center.x < l2.center.x)
            {
                left_pair = l1, right_pair = l2;
            }
            else
            {
                left_pair = l2, right_pair = l1;
            }
            // 计算装甲板的中心点
            center = (left_pair.center + right_pair.center) / 2;
        }

        // 装甲板的左右光源
        Pair left_pair, right_pair;
        // 装甲板的中心点
        cv::Point2f center;

    };

    class Detector
    {
    public:
        struct LightParams
        {
            double min_ratio; // 宽高比的最小值
            double max_ratio; // 宽高比的最大值
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
        std::vector<InsideBox> matchLights(const std::vector<Pair> &lights);

        // 二值化阈值
        int binary_thres;

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
