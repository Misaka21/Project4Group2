#include "Detector.h"
#include "opencv2/imgproc.hpp"

namespace ImgProcess {

// Detector类的构造函数
    Detector::Detector(
            const int &bin_thres,
            const LightParams &l,                                      // 光源参数
            const ArmorParams &a)                                      // 装甲板参数
            : binary_thres(bin_thres),  l(l), a(a)                      // 初始化成员变量
    {
    }
    // 检测函数，接收一个图像，返回检测到的装甲板集合
    std::vector<InsideBox> Detector::detect(const cv::Mat &input)
    {
        binary_img = preprocessImage(input);
        lights_ = findPairs(binary_img); // 在二值图中找到光源
        armors_ = matchLights(lights_);          // 匹配光源，识别出装甲板

        return armors_; // 返回检测到的装甲板集合
    }
    // 图像预处理函数，接收RGB图像，返回二值图
    cv::Mat Detector::preprocessImage(const cv::Mat &gray_img)
    {
        cv::Mat binary_img;                                                        // 定义二值图
        cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY); // 应用二值化

        return binary_img; // 返回二值图
    }

    std::vector<Pair> Detector::findPairs(const cv::Mat &binary_img)
    {
        using std::vector;
        vector<vector<cv::Point>> contours;                                                            // 定义轮廓集合
        vector<cv::Vec4i> hierarchy;                                                                   // 定义层级关系
        cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 查找轮廓

        vector<Pair> Pairs;            // 定义光源集合

        for (const auto &contour : contours)
        { // 遍历所有轮廓
            if (contour.size() < 5)
                continue; // 如果轮廓点数小于5，跳过

            auto r_rect = cv::minAreaRect(contour); // 计算轮廓的最小外接矩形
            auto pair = Pair(r_rect);             // 创建Light对象

            if (isPair(pair))
            {                                   // 判断是否为光源
                auto rect = pair.boundingRect(); // 获取光源的边界矩形
                if (0 <= rect.x && 0 <= rect.width && rect.x + rect.width <= binary_img.cols && 0 <= rect.y &&
                    0 <= rect.height && rect.y + rect.height <= binary_img.rows)
                {
                    auto roi = binary_img(rect); // 获取矩形区域的图像
                    pair.type = roi.at<uchar>(roi.rows / 2, 0) > roi.at<uchar>(roi.rows / 2, roi.cols)  ? LEFT : RIGHT;
                    Pairs.emplace_back(pair); // 将光源添加到集合中
                }
            }
        }

        return Pairs; // 返回光源集合
    }
    // 检查给定的光源对象是否符合设定的光源特征
    bool Detector::isPair(const Pair &pair)
    {
        // 计算光源的长宽比（短边/长边）
        float ratio = pair.width / pair.length;
        // 检查长宽比是否在预设的范围内
        bool ratio_ok = l.min_ratio < ratio && ratio < l.max_ratio;


        // 判断该光源是否有效，需要同时满足比例和角度条件
        bool is_light = ratio_ok ;

        return is_light; // 返回是否为有效光源的判断结果
    }
    // 匹配光源对，尝试找出可能的装甲板
    std::vector<InsideBox> Detector::matchLights(const std::vector<Pair> &lights)
    {
        std::vector<InsideBox> insideBoxes;       // 定义装甲板列表

        // 遍历所有可能的光源对
        for (auto light_1 = lights.begin(); light_1 != lights.end(); light_1++)
        {
            for (auto light_2 = light_1 + 1; light_2 != lights.end(); light_2++)
            {
                // 如果两光源之间存在其他光源，则跳过
                if (containLight(*light_1, *light_2, lights))
                {
                    continue;
                }

                // 判断这两个光源是否能构成一个有效的装甲板
                auto type = isInsideBox(*light_1, *light_2);
                // 如果装甲板有效，则加入装甲板列表
                if (type)
                {
                    auto insideBox = InsideBox(*light_1, *light_2);
                    insideBoxes.emplace_back(insideBox); // 将装甲板加入列表
                }
            }
        }

        return insideBoxes; // 返回识别到的装甲板列表
    }
    bool Detector::containLight(
            const Pair &light_1, const Pair &light_2, const std::vector<Pair> &lights) {
        // 获取两个光源的顶部和底部坐标点
        auto points = std::vector<cv::Point2f>{light_1.top, light_1.bottom, light_2.top, light_2.bottom};
        // 计算这些点形成的边界矩形
        auto bounding_rect = cv::boundingRect(points);

        // 遍历所有光源，检查它们是否位于上述边界矩形内
        for (const auto &test_light: lights) {
            // 跳过参与形成边界矩形的两个光源
            if (test_light.center == light_1.center || test_light.center == light_2.center)
                continue;

            // 如果任何光源的顶部、底部或中心位于边界矩形内，则返回true
            if (
                    bounding_rect.contains(test_light.top) || bounding_rect.contains(test_light.bottom) ||
                    bounding_rect.contains(test_light.center)) {
                return true;
            }
        }
        return false; // 如果没有其他光源在边界矩形内，则返回false
    }
    // 判断两个光源是否构成装甲板，并返回装甲板类型
    bool Detector::isInsideBox(const Pair &light_1, const Pair &light_2)
    {
        // 计算两个光源长度的比例（短边/长边）
        float light_length_ratio = light_1.length < light_2.length ? light_1.length / light_2.length
                                                                   : light_2.length / light_1.length;
        // 检查光源长度比例是否符合预设条件
        bool light_ratio_ok = light_length_ratio > a.min_pair_ratio;

        // 计算两个光源中心点之间的距离（单位：光源长度）
        float avg_light_length = (light_1.length + light_2.length) / 2;
        float center_distance = cv::norm(light_1.center - light_2.center) / avg_light_length;
        // 检查中心距是否符合装甲板的大小条件
        bool center_distance_ok = (a.min_center_distance <= center_distance &&
                                   center_distance < a.max_center_distance);


        // 综合以上条件判断是否构成装甲板
        bool is_armor = light_ratio_ok && center_distance_ok;

        return is_armor; // 返回装甲板类型
    }
}
