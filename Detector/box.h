//
// Created by admin on 24-4-26.
//

#ifndef GROUP2_BOX_H
#define GROUP2_BOX_H


#include <opencv2/core.hpp>
#include "opencv2/highgui.hpp"

#include "opencv2/imgproc.hpp"

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
            // 计算光源的长度和宽度
            width = cv::norm(left - right);

            std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
            { return a.y < b.y; });
            top = (p[0] + p[1]) / 2;
            bottom = (p[2] + p[3]) / 2;
            length = cv::norm(top -bottom);
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



}



#endif //GROUP2_BOX_H
