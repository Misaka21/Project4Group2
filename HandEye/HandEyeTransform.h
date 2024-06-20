//
// Created by david on 24-6-16.
//

#ifndef GROUP2_HANDEYETRANSFORM_H
#define GROUP2_HANDEYETRANSFORM_H



#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/core/types_c.h"
#include <iostream>
#include <sstream>
#include <Eigen/Dense>
#include <fstream>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace Transform {
	class Calib{
	public:
		Calib();
		void detectboard();
		void calibfunc();
		void getimg(cv::Mat image);
		cv::Mat img;
	private:
		float cam_x, cam_y;    // 相机坐标
		float arm_x, arm_y;    // 机械臂坐标

		std::vector<std::pair<Eigen::Matrix<float, 2, 1>, Eigen::Matrix<float, 2, 1>>> coords; // 存储所有坐标


		int n;
		void saveToYAML();

		void linear_regression(const std::vector<std::pair<Eigen::Matrix<float, 2, 1>, Eigen::Matrix<float, 2, 1>>>& tmpcoord,
		                       Eigen::Vector2f& camera_fit_params,
		                       Eigen::Vector2f& arm_fit_params);
	};


} // Transform

#endif //GROUP2_HANDEYETRANSFORM_H
