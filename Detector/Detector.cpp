#include <iostream>
#include "Detector.h"

namespace ImgProcess {


    InsideDetector::InsideDetector(
            const int &bin_thres,
            const PairParams &p,
            const InsideBoxParams &a)
            : binary_thres(bin_thres), p(p), a(a)
    {
    }

    std::vector<InsideBox> InsideDetector::detect(const cv::Mat &input)
    {
        binary_img = preprocessImage(input);


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
                cv::circle(Draw, vertices[i], 5, cv::Scalar(0, 0, 255), 5);
        }

        _insideboxes = matchPairs(_pairs);


        for (const auto& InsideBox : _insideboxes) {

            cv::Point2f vertices[4];
            vertices[0]=InsideBox.left_pair.top;
            vertices[3]=InsideBox.left_pair.bottom;
            vertices[1]=InsideBox.right_pair.top;
            vertices[2]=InsideBox.right_pair.bottom;
//            InsideBox.right_pair.points(vertices + 2);
            std::cout<<"[DEBUG]: Inside"<<vertices[0]<<vertices[3]<<vertices[1]<<vertices[2]<<std::endl;

            cv::Rect InsideBox_rect(cv::boundingRect(cv::Mat(4, 1, CV_32FC2, vertices)));


            cv::rectangle(Draw, InsideBox_rect, cv::Scalar(255, 255, 255), 10);


            cv::circle(Draw, InsideBox.center, 10, cv::Scalar(0, 0, 255), -1);
        }
        cv::namedWindow("binar", cv::WINDOW_NORMAL);
        cv::resizeWindow("binar", 1000, 1000);
        cv::imshow("binar",Draw);
        //cv::waitKey(1);
        return _insideboxes;
    }


    cv::Mat InsideDetector::preprocessImage(const cv::Mat &gray_img)
    {
        cv::Mat binary_img;
        cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY);

        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::dilate(binary_img, binary_img, element);
        cv::erode(binary_img, binary_img, element);

        return binary_img;
    }

    std::vector<Pair> InsideDetector::findPairs(const cv::Mat &binary_img)
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
                    //std::cout<<"fuck"<<pair.type<<std::endl;
                    cv::waitKey(1);
                    Pairs.emplace_back(pair);

                }
            }
        }

        return Pairs;
    }


    bool InsideDetector::isPair(const Pair &possible_pair)
    {
        float ratio = possible_pair.width / possible_pair.length;
        float area = possible_pair.length * possible_pair.width;

        bool ratio_ok = p.min_ratio < ratio && ratio < p.max_ratio;

        bool area_ok= p.min_area < area && area < p.max_area;

        bool is_pair = ratio_ok && area_ok;

        return is_pair;
    }


    std::vector<InsideBox> InsideDetector::matchPairs(std::vector<Pair> &pairs)
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
        auto compare2 = [](const InsideBox& a, const InsideBox& b) {
            return a.center.x < b.center.x;
        };

        std::sort(insideBoxes.begin(), insideBoxes.end(), compare2);

        return insideBoxes;
    }
    bool InsideDetector::containPair(
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


    bool InsideDetector::isInsideBox(const Pair &pair_1, const Pair &pair_2)
    {
        float pair_length_ratio = pair_1.length < pair_2.length ? pair_1.length / pair_2.length
                                                                : pair_2.length / pair_1.length;
        //std::cout << "pair_length_ratio" << pair_length_ratio << std::endl;

        bool pair_ratio_ok = pair_length_ratio > a.min_pair_ratio;

        float avg_pair_length = (pair_1.length + pair_2.length) / 2;
        float center_distance = cv::norm(pair_1.center - pair_2.center);

        bool center_distance_ok = (a.min_center_distance <= center_distance &&
                                   center_distance < a.max_center_distance);
        bool center_ok= pair_1.center.x < pair_2.center.x;
        bool type_ok= pair_1.type == LEFT && pair_2.type == RIGHT;

        bool is_insidebox = pair_ratio_ok && center_distance_ok && type_ok && center_ok;
        //std::cout << pair_1.type << pair_2.type << std::endl;
        //std::cout<<is_insidebox<<std::endl;
        return is_insidebox;
    }
    auto OutsideDetector::thresholdBookmark(const cv::Mat& img) ->  cv::Mat {
        cv::Mat result;
        //cv::cvtColor(imgContrast, imgContrast, cv::COLOR_BGR2GRAY);
        cv::threshold(img, result, 200, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);

        return result;
    }

    auto OutsideDetector::outsideprocess(const cv::Mat& img) -> std::vector<outsidemarkpoint> {

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::Mat iimage = img.clone(); // Clone to avoid modifying the original image
        cv::threshold(iimage, iimage, 175, 255, cv::THRESH_BINARY);

        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::dilate(iimage, iimage, element);
        cv::erode(iimage, iimage, element);
        //cv::namedWindow("binary_img", cv::WINDOW_NORMAL);
        //cv::resizeWindow("binary_img", 1000, 1000);
        //cv::imshow("binary_img",iimage);
        // 查找轮廓
        cv::findContours(iimage, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

        std::vector<outsidemarkpoint> outsidePoints;

        for (int idx = 0; idx < hierarchy.size(); ++idx) {
            // 找到既没有子轮廓也没有父轮廓的轮廓
            if (hierarchy[idx][2] == -1 && hierarchy[idx][3] == -1) {
                cv::RotatedRect rotatedRect = cv::minAreaRect(contours[idx]);

                // 基于轮廓面积过滤不符合大小要求的矩形
                if (rotatedRect.size.area() < this->_MIN_Area || rotatedRect.size.area() > this->_MAX_Area)
                    continue;

                cv::Point2f rectPoints[4];
                rotatedRect.points(rectPoints); // 获取旋转矩形的四个角点

                // 排序和调整rectPoints以匹配outsidemarkpoint结构
                std::sort(rectPoints, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.x < b.x;
                });

                // 对左侧两点和右侧两点分别按y值排序
                std::sort(rectPoints, rectPoints + 2, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.y < b.y;
                });
                std::sort(rectPoints + 2, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.y < b.y;
                });

                // 将排序后的点添加到结果向量中
                outsidemarkpoint points = { rectPoints[0], rectPoints[1], rectPoints[2], rectPoints[3] };
                std::cout<<"[DEBUG]: Outside"<<points.l1<<points.l2<<points.r1<<points.r2<<std::endl;
                outsidePoints.push_back(points);
            }
        }

        // 根据矩形的左上角点从左到右排序
        std::sort(outsidePoints.begin(), outsidePoints.end(), [](const outsidemarkpoint& a, const outsidemarkpoint& b) {
            return a.l1.x < b.l1.x;
        });

        return outsidePoints;
    }
    void OutsideDetector::drawPoints(const cv::Mat& img,
                                     const std::vector<outsidemarkpoint>& outsidePoints,
                                     const std::string& winname) {
        cv::Mat frame= img.clone();
        // 为外部点绘制
        for (const auto& point : outsidePoints) {
            // 使用红色标记外部点
            drawPoint(frame, point, cv::Scalar(0, 0, 255));
        }

        // 显示带有点和坐标的图像
        cv::namedWindow(winname, cv::WINDOW_FREERATIO);
        cv::resizeWindow(winname, cv::Size(720, 540));
        cv::imshow(winname, frame);
        cv::waitKey(1);
    }


    template <typename T>
    void OutsideDetector::drawPoint(cv::Mat& img, const T& point, const cv::Scalar& color) {
        // 绘制每个点
        cv::circle(img, point.l1, 5, color, -1); // 左上角点
        cv::circle(img, point.l2, 5, color, -1); // 左下角点
        cv::circle(img, point.r1, 5, color, -1); // 右上角点
        cv::circle(img, point.r2, 5, color, -1); // 右下角点

        // 按顺序连起来
        cv::line(img, point.l1, point.r1, color, 2);
        cv::line(img, point.r1, point.r2, color, 2);
        cv::line(img, point.r2, point.l2, color, 2);
        cv::line(img, point.l2, point.l1, color, 2);

        // 标出每个点的坐标
        std::ostringstream l1Text, l2Text, r1Text, r2Text;
        l1Text << "(" << point.l1.x << "," << point.l1.y << ")";
        l2Text << "(" << point.l2.x << "," << point.l2.y << ")";
        r1Text << "(" << point.r1.x << "," << point.r1.y << ")";
        r2Text << "(" << point.r2.x << "," << point.r2.y << ")";

        cv::putText(img, l1Text.str(), point.l1, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, l2Text.str(), point.l2, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, r1Text.str(), point.r1, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, r2Text.str(), point.r2, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        // 计算中心点坐标
        cv::Point2f center = (point.l1 + point.r2) * 0.5f;
        std::ostringstream centerText;
        centerText << "Center: (" << center.x << "," << center.y << ")";


        // 计算并显示矩形的大小（宽度和高度）
        float width = cv::norm(point.l1 - point.r1);
        float height = cv::norm(point.l1 - point.l2);
        std::ostringstream sizeText;
        sizeText << "Size: " << width << "x" << height << "=" << width * height;

        cv::circle(img, center, 5, cv::Scalar(0, 255, 255), -1); // 右下角点
        cv::Point2f sizePos = center + cv::Point2f(20, 20); // 将大小信息位置稍微上移，以免与中心点坐标重叠

        cv::putText(img, centerText.str(), sizePos, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2); // 使用洋红色标出中心点坐标
        cv::putText(img, sizeText.str(), sizePos + cv::Point2f(0, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2); // 使用洋红色标出矩形大小

    }




}
