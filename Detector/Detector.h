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
        struct PairParams
        {
            double min_ratio;
            double max_ratio;
            double min_area;
            double max_area;
        };

        struct InsideBoxParams
        {
            double min_pair_ratio;
            double min_center_distance;
            double max_center_distance;
        };

        Detector(const int &bin_thres, const PairParams &p, const InsideBoxParams &a);

        std::vector<InsideBox> detect(const cv::Mat &input);

        cv::Mat preprocessImage(const cv::Mat &input);

        std::vector<Pair> findPairs(const cv::Mat &binary_img);

        std::vector<InsideBox> matchPairs(std::vector<Pair> &pairs);

        int binary_thres=220;


        PairParams p;

        InsideBoxParams a;

        cv::Mat binary_img;
    private:
        bool isPair(const Pair &possible_pair);

        bool containPair(
                const Pair &pair_1, const Pair &pair_2, const std::vector<Pair> &pairs);

        bool isInsideBox(const Pair &pair_1, const Pair &pair_2);

        std::vector<Pair> _pairs;

        // 存储匹配成功的装甲板
        std::vector<InsideBox> _insideboxes;
    };

}
#endif //RM2024_DETECTOR_H
