#pragma once

#ifndef RM2024_DETECTOR_H
#define RM2024_DETECTOR_H

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

// STD
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>

#define MIN_AREA 200000 // 定义最小面积阈值
#define MAX_AREA 3000000 // 定义最大面积阈值
#define DILATE 3 // 定义膨胀系数
#define ERODE 1  // 定义腐蚀系数

typedef struct
{
	cv::Point2f l1, l2, r1, r2;
}insidemarkpoint;
typedef struct
{
	cv::Point2f l1, l2, r1, r2;
}outsidemarkpoint;
namespace Detect {

	class ImgProcessor
	{
	public:
		auto thresholdBookmark(const cv::Mat& img) -> cv::Mat;
		auto insideprocess(const cv::Mat& img) -> std::vector<insidemarkpoint>;
		auto outsideprocess(const cv::Mat& img) -> std::vector<outsidemarkpoint>;

		void drawPoints(const cv::Mat& img,
			const std::vector<insidemarkpoint>& insidePoints,
			const std::vector<outsidemarkpoint>& outsidePoints,
			const std::string& winname);
	private:
		template <typename T>
		void drawPoint(cv::Mat& img, const T& point, const cv::Scalar& color);
	};
}




#endif //RM2024_DETECTOR_H
