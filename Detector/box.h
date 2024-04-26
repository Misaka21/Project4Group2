//
// Created by admin on 24-4-26.
//

#ifndef GROUP2_BOX_H
#define GROUP2_BOX_H


#include <opencv2/core.hpp>
#include "opencv2/highgui.hpp"

#include "opencv2/imgproc.hpp"

namespace ImgProcess {
    const int LEFT = 0;
    const int RIGHT = 1;
    struct Pair : public cv::RotatedRect
    {
        Pair() = default;

        explicit Pair(cv::RotatedRect box) : cv::RotatedRect(box)
        {
            cv::Point2f p[4];
            box.points(p);
            std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
            { return a.x < b.x; });
            left = (p[0] + p[1]) / 2;
            right = (p[2] + p[3]) / 2;
            width = cv::norm(left - right);

            std::sort(p, p + 4, [](const cv::Point2f &a, const cv::Point2f &b)
            { return a.y < b.y; });
            top = (p[0] + p[1]) / 2;
            bottom = (p[2] + p[3]) / 2;
            length = cv::norm(top -bottom);
            tilt_angle = std::atan2(std::abs(left.x - right.x), std::abs(left.y - right.y));
            tilt_angle = tilt_angle / CV_PI * 180;
        }

        int type;
        cv::Point2f left, right,top,bottom;
        double length;
        double width;
        double tilt_angle;
    };
    struct InsideBox
    {
        InsideBox() = default;
        InsideBox(const Pair &l1, const Pair &l2)
        {

            if (l1.center.x < l2.center.x)
            {
                left_pair = l1, right_pair = l2;
            }
            else
            {
                left_pair = l2, right_pair = l1;
            }
            center = (left_pair.center + right_pair.center) / 2;
        }

        Pair left_pair, right_pair;
        cv::Point2f center;
    };



}



#endif //GROUP2_BOX_H
