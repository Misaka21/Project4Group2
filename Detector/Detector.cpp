#include "Detector.h"
namespace Detect {
	auto ImgProcessor::thresholdBookmark(const cv::Mat& img) -> cv::Mat {
		//增加对比度
		cv::Mat imgContrast, result;
		double alpha = 1.5; // 对比度控制系数
		int beta = 0; // 亮度控制值
		img.convertTo(imgContrast, -1, alpha, beta); // 应用对比度和亮度调整
		//二值化
		cv::cvtColor(imgContrast, imgContrast, cv::COLOR_BGR2GRAY);
		cv::threshold(imgContrast, result, 100, 255, cv::THRESH_BINARY);
		// 膨胀操作
		if (DILATE > 0) {
			cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::dilate(result, result, dilateElement, cv::Point(-1, -1), DILATE);
		}

		// 腐蚀操作
		if (ERODE > 0) {
			cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::erode(result, result, erodeElement, cv::Point(-1, -1), ERODE);
		}
		return result;
	}
	auto ImgProcessor::insideprocess(const cv::Mat& img) -> std::vector<insidemarkpoint> {
		//inside是指内部的书签盒，它有嵌套的两个轮廓，这里返回外轮廓的点
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::Mat iimage = img;
		// 查找轮廓
		cv::findContours(iimage, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

		std::vector<insidemarkpoint> insidePoints;

		for (int idx = 0; idx < hierarchy.size(); idx = hierarchy[idx][0])
		{
			if (hierarchy[idx][2] < 0 && hierarchy[idx][3] == -1) // 跳过没有子轮廓的轮廓
				continue;

			cv::RotatedRect rotatedRect = cv::minAreaRect(contours[idx]);

			// 确保旋转矩形的面积在预期范围内
			if (rotatedRect.size.area() < MIN_AREA || rotatedRect.size.area() > MAX_AREA) continue;


			cv::Point2f rectPoints[4];
			rotatedRect.points(rectPoints); // 获取旋转矩形的四个角点

			// 确保角点从左到右排序
			std::sort(rectPoints, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
				return a.x < b.x;
				});

			// 对左侧两点和右侧两点分别按y值排序，确保l1, l2是左上和左下，r1, r2是右上和右下
			std::sort(rectPoints, rectPoints + 2, [](const cv::Point2f& a, const cv::Point2f& b) {
				return a.y < b.y;
				});
			std::sort(rectPoints + 2, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
				return a.y < b.y;
				});

			insidemarkpoint points = { rectPoints[0], rectPoints[1], rectPoints[2], rectPoints[3] };
			insidePoints.push_back(points);
		}

		// 根据矩形的左上角点从左到右排序
		std::sort(insidePoints.begin(), insidePoints.end(), [](const insidemarkpoint& a, const insidemarkpoint& b) {
			return a.l1.x < b.l1.x;
			});
		return insidePoints;
	}
	auto ImgProcessor::outsideprocess(const cv::Mat& img) -> std::vector<outsidemarkpoint> {
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::Mat iimage = img.clone(); // Clone to avoid modifying the original image

		// 查找轮廓
		cv::findContours(iimage, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

		std::vector<outsidemarkpoint> outsidePoints;

		for (int idx = 0; idx < hierarchy.size(); ++idx) {
			// 找到既没有子轮廓也没有父轮廓的轮廓
			if (hierarchy[idx][2] == -1 && hierarchy[idx][3] == -1) {
				cv::RotatedRect rotatedRect = cv::minAreaRect(contours[idx]);

				// 基于轮廓面积过滤不符合大小要求的矩形
				if (rotatedRect.size.area() < MIN_AREA || rotatedRect.size.area() > MAX_AREA)
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
				outsidePoints.push_back(points);
			}
		}

		// 根据矩形的左上角点从左到右排序
		std::sort(outsidePoints.begin(), outsidePoints.end(), [](const outsidemarkpoint& a, const outsidemarkpoint& b) {
			return a.l1.x < b.l1.x;
			});

		return outsidePoints;
	}
	void ImgProcessor::drawPoints(const cv::Mat& img,
		const std::vector<insidemarkpoint>& insidePoints,
		const std::vector<outsidemarkpoint>& outsidePoints,
		const std::string& winname) {
		cv::Mat frame= img.clone();
		// 为外部点绘制
		for (const auto& point : outsidePoints) {
			// 使用红色标记外部点
			drawPoint(frame, point, cv::Scalar(0, 0, 255));
		}
		// 为内部点绘制
		for (const auto& point : insidePoints) {
			// 使用绿色标记内部点
			drawPoint(frame, point, cv::Scalar(0, 255, 0));
		}



		// 显示带有点和坐标的图像
		cv::imshow(winname, frame);
		cv::waitKey(1);
	}


	template <typename T>
	void ImgProcessor::drawPoint(cv::Mat& img, const T& point, const cv::Scalar& color) {
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
