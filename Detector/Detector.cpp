#include <iostream>
#include "Detector.h"

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
        cv::imshow("binary_img",binary_img);
        lights_ = findPairs(binary_img); // 在二值图中找到光源
        cv::Mat Draw=input.clone();
        for (const auto& light : lights_) {
            // 获取旋转矩形的四个顶点
            cv::Point2f vertices[4];
            light.points(vertices);

            // 绘制旋转矩形的边界
            for (int i = 0; i < 4; i++)
                cv::line(Draw, vertices[i], vertices[(i+1)%4], cv::Scalar(255, 255, 255));

            // 计算并标出长宽比
            std::string ratio = std::to_string(light.width * light.length);
            cv::putText(Draw, ratio, light.center, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0));

            // 标出旋转矩形的顶点
            for (int i = 0; i < 4; i++)
                cv::circle(Draw, vertices[i], 5, cv::Scalar(0, 0, 255), -1);
        }

        armors_ = matchLights(lights_);          // 匹配光源，识别出装甲板
        for (const auto& armor : armors_) {
            // 获取装甲板的左右光源的四个顶点
            cv::Point2f vertices[4];
            armor.left_pair.points(vertices);
            armor.right_pair.points(vertices+2);

            // 计算装甲板的边界矩形
            cv::Rect armor_rect(cv::boundingRect(cv::Mat(4, 1, CV_32FC2, vertices)));

            // 绘制装甲板的边界
            cv::rectangle(Draw, armor_rect, cv::Scalar(0, 255, 0), 2);

            // 标出装甲板的中心点
            cv::circle(Draw, armor.center, 10, cv::Scalar(0, 0, 255), -1);
        }
        //cv::namedWindow("binar", cv::WINDOW_NORMAL);
        //cv::resizeWindow("binar", 1000, 1000);
        cv::imshow("binar",Draw);
        cv::waitKey(1);
        return armors_; // 返回检测到的装甲板集合
    }
    // 图像预处理函数，接收RGB图像，返回二值图
    cv::Mat Detector::preprocessImage(const cv::Mat &gray_img)
    {
        cv::Mat binary_img;                                                        // 定义二值图
        cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY); // 应用二值化
// 创建一个5x5的结构元素
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

// 应用膨胀操作
        cv::dilate(binary_img, binary_img, element);

// 应用腐蚀操作
        cv::erode(binary_img, binary_img, element);
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
                    cv::imshow("roi",roi);

                    // 获取左边10x10的子区域
                    cv::Rect left_roi(0, roi.rows / 2 - 10, 20, 20);
                    cv::Mat left_sub = roi(left_roi);
                    cv::imshow("roil",left_sub);

                    // 获取右边10x10的子区域
                    cv::Rect right_roi(roi.cols - 20, roi.rows / 2 - 10, 20, 20);
                    cv::Mat right_sub = roi(right_roi);
                    cv::imshow("roir",right_sub);
                    // 计算子区域的平均值
                    cv::Scalar left_mean = cv::mean(left_sub);
                    cv::Scalar right_mean = cv::mean(right_sub);

// 比较平均值
                    pair.type = left_mean[0] > right_mean[0] ? LEFT : RIGHT;
                    std::cout<<"fuck"<<pair.type<<std::endl;
                    cv::waitKey(1);
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
        float area =pair.length*pair.width;
        // 检查长宽比是否在预设的范围内
        bool ratio_ok = l.min_ratio < ratio && ratio < l.max_ratio;

        bool area_ok=l.min_area < area && area < l.max_area;

        // 判断该光源是否有效，需要同时满足比例和角度条件
        bool is_light = ratio_ok &&area_ok;

        return is_light; // 返回是否为有效光源的判断结果
    }
    // 匹配光源对，尝试找出可能的装甲板
    std::vector<InsideBox> Detector::matchLights(std::vector<Pair> &lights)
    {
        std::vector<InsideBox> insideBoxes;       // 定义装甲板列表

        auto compare = [](const Pair &a, const Pair &b) {
            return a.top.x < b.top.x;
        };

// 对insideBoxes进行排序
        std::sort(lights.begin(), lights.end(), compare);
        // 遍历所有可能的光源对
        for (auto light_1 = lights.begin(); light_1 != lights.end(); light_1++)
        {
            if(light_1->type==RIGHT) continue;
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
                    //std::cout<<"sorted"<<insideBox.left_pair.type<<insideBox.right_pair.type<<std::endl;
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
        std::cout<<"light_length_ratio"<<light_length_ratio<<std::endl;
        // 检查光源长度比例是否符合预设条件
        bool light_ratio_ok = light_length_ratio > a.min_pair_ratio;

        // 计算两个光源中心点之间的距离（单位：光源长度）
        float avg_light_length = (light_1.length + light_2.length) / 2;
        float center_distance = cv::norm(light_1.center - light_2.center);
        // 检查中心距是否符合装甲板的大小条件
        bool center_distance_ok = (a.min_center_distance <= center_distance &&
                                   center_distance < a.max_center_distance);
        bool center_ok=light_1.center.x< light_2.center.x;
        bool type_ok=light_1.type==LEFT&&light_2.type==RIGHT;
        // 综合以上条件判断是否构成装甲板
        bool is_armor = light_ratio_ok && center_distance_ok&&type_ok&&center_ok;
        std::cout<<light_1.type<<light_2.type<<std::endl;
        //std::cout<<is_armor<<std::endl;
        return is_armor; // 返回装甲板类型
    }
}
