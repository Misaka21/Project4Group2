#include <iostream>
#include "Detector.h"

namespace ImgProcess {


    Detector::Detector(
            const int &bin_thres,
            const PairParams &p,
            const InsideBoxParams &a)
            : binary_thres(bin_thres), p(p), a(a)
    {
    }

    std::vector<InsideBox> Detector::detect(const cv::Mat &input)
    {
        binary_img = preprocessImage(input);
        cv::imshow("binary_img",binary_img);
        _pairs = findPairs(binary_img);
        cv::Mat Draw=input.clone();
        for (const auto& light : _pairs) {

            cv::Point2f vertices[4];
            light.points(vertices);


            for (int i = 0; i < 4; i++)
                cv::line(Draw, vertices[i], vertices[(i+1)%4], cv::Scalar(255, 255, 255));


            std::string ratio = std::to_string(light.width * light.length);
            cv::putText(Draw, ratio, light.center, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0));


            for (int i = 0; i < 4; i++)
                cv::circle(Draw, vertices[i], 5, cv::Scalar(0, 0, 255), -1);
        }

        _insideboxes = matchPairs(_pairs);
        for (const auto& InsideBox : _insideboxes) {

            cv::Point2f vertices[4];
            InsideBox.left_pair.points(vertices);
            InsideBox.right_pair.points(vertices + 2);


            cv::Rect InsideBox_rect(cv::boundingRect(cv::Mat(4, 1, CV_32FC2, vertices)));


            cv::rectangle(Draw, InsideBox_rect, cv::Scalar(0, 255, 0), 2);


            cv::circle(Draw, InsideBox.center, 10, cv::Scalar(0, 0, 255), -1);
        }
        //cv::namedWindow("binar", cv::WINDOW_NORMAL);
        //cv::resizeWindow("binar", 1000, 1000);
        cv::imshow("binar",Draw);
        cv::waitKey(1);
        return _insideboxes;
    }
    // 图像预处理函数，接收RGB图像，返回二值图
    cv::Mat Detector::preprocessImage(const cv::Mat &gray_img)
    {
        cv::Mat binary_img;
        cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY);

        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::dilate(binary_img, binary_img, element);
        cv::erode(binary_img, binary_img, element);

        return binary_img; // 返回二值图
    }

    std::vector<Pair> Detector::findPairs(const cv::Mat &binary_img)
    {
        using std::vector;
        vector<vector<cv::Point>> contours;
        vector<cv::Vec4i> hierarchy;
        cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        vector<Pair> Pairs;

        for (const auto &contour : contours)
        {
            if (contour.size() < 5)
                continue;

            auto r_rect = cv::minAreaRect(contour);
            auto pair = Pair(r_rect);

            if (isPair(pair))
            {
                auto rect = pair.boundingRect();
                if (0 <= rect.x && 0 <= rect.width && rect.x + rect.width <= binary_img.cols && 0 <= rect.y &&
                    0 <= rect.height && rect.y + rect.height <= binary_img.rows)
                {
                    auto roi = binary_img(rect);
                    cv::imshow("roi",roi);

                    cv::Rect left_roi(0, roi.rows / 2 - 10, 20, 20);
                    cv::Mat left_sub = roi(left_roi);
                    cv::imshow("roil",left_sub);


                    cv::Rect right_roi(roi.cols - 20, roi.rows / 2 - 10, 20, 20);
                    cv::Mat right_sub = roi(right_roi);
                    cv::imshow("roir",right_sub);

                    cv::Scalar left_mean = cv::mean(left_sub);
                    cv::Scalar right_mean = cv::mean(right_sub);

                    pair.type = left_mean[0] > right_mean[0] ? LEFT : RIGHT;
                    std::cout<<"fuck"<<pair.type<<std::endl;
                    cv::waitKey(1);
                    Pairs.emplace_back(pair); // 将光源添加到集合中

                }
            }
        }

        return Pairs;
    }


    bool Detector::isPair(const Pair &possible_pair)
    {
        float ratio = possible_pair.width / possible_pair.length;
        float area = possible_pair.length * possible_pair.width;

        bool ratio_ok = p.min_ratio < ratio && ratio < p.max_ratio;

        bool area_ok= p.min_area < area && area < p.max_area;

        bool is_pair = ratio_ok && area_ok;

        return is_pair;
    }


    std::vector<InsideBox> Detector::matchPairs(std::vector<Pair> &pairs)
    {
        std::vector<InsideBox> insideBoxes;

        auto compare = [](const Pair &a, const Pair &b) {
            return a.top.x < b.top.x;
        };


        std::sort(pairs.begin(), pairs.end(), compare);

        for (auto pair_1 = pairs.begin(); pair_1 != pairs.end(); pair_1++)
        {
            if(pair_1->type == RIGHT) continue;
            for (auto pair_2 = pair_1 + 1; pair_2 != pairs.end(); pair_2++)
            {

                if (containPair(*pair_1, *pair_2, pairs))
                {
                    continue;
                }


                auto type = isInsideBox(*pair_1, *pair_2);

                if (type)
                {
                    auto insideBox = InsideBox(*pair_1, *pair_2);
                    //std::cout<<"sorted"<<insideBox.left_pair.type<<insideBox.right_pair.type<<std::endl;
                    insideBoxes.emplace_back(insideBox);
                }
            }
        }

        return insideBoxes;
    }
    bool Detector::containPair(
            const Pair &pair_1, const Pair &pair_2, const std::vector<Pair> &pairs) {

        auto points = std::vector<cv::Point2f>{pair_1.top, pair_1.bottom, pair_2.top, pair_2.bottom};

        auto bounding_rect = cv::boundingRect(points);


        for (const auto &test_pair: pairs) {
            if (test_pair.center == pair_1.center || test_pair.center == pair_2.center)
                continue;

            if (
                    bounding_rect.contains(test_pair.top) || bounding_rect.contains(test_pair.bottom) ||
                    bounding_rect.contains(test_pair.center)) {
                return true;
            }
        }
        return false;
    }


    bool Detector::isInsideBox(const Pair &pair_1, const Pair &pair_2)
    {
        float pair_length_ratio = pair_1.length < pair_2.length ? pair_1.length / pair_2.length
                                                                : pair_2.length / pair_1.length;
        std::cout << "pair_length_ratio" << pair_length_ratio << std::endl;

        bool pair_ratio_ok = pair_length_ratio > a.min_pair_ratio;

        float avg_pair_length = (pair_1.length + pair_2.length) / 2;
        float center_distance = cv::norm(pair_1.center - pair_2.center);

        bool center_distance_ok = (a.min_center_distance <= center_distance &&
                                   center_distance < a.max_center_distance);
        bool center_ok= pair_1.center.x < pair_2.center.x;
        bool type_ok= pair_1.type == LEFT && pair_2.type == RIGHT;

        bool is_insidebox = pair_ratio_ok && center_distance_ok && type_ok && center_ok;
        std::cout << pair_1.type << pair_2.type << std::endl;
        //std::cout<<is_insidebox<<std::endl;
        return is_insidebox;
    }
}
