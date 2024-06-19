//
// Created by david on 24-6-16.
//

#include "HandEyeTransform.h"

namespace Transform {


	void calib::detectboard(){
		// 检测棋盘格角点
		cv::Mat grayImage=this->img.clone(),image=this->img.clone();
		std::vector<cv::Point2f> corners;  // 存储角点
		int board_width = 9;               // 标定板宽度角点数
		int board_height = 6;              // 标定板高度角点数
		cv::Size board_size(board_width, board_height);

		bool found = cv::findChessboardCorners(grayImage, board_size, corners,
		                                       cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

		if (found) {
			// 优化角点位置
			cv::cornerSubPix(grayImage, corners, cv::Size(11,11), cv::Size(-1,-1),
			                 cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));

			// 显示角点和标注坐标
			cv::drawChessboardCorners(image, board_size, corners, found);

			for (int i = 0; i < corners.size(); ++i) {
				// 标注图像上的信息
				std::stringstream ss;
				ss << "#" << i+1 << ": (" << corners[i].x << ", " << corners[i].y << ")";
				cv::putText(image, ss.str(), corners[i] + cv::Point2f(5,5), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,255,0), 1);

				// 控制台输出坐标信息
				std::cout << "角点 " << i+1 << ": (" << corners[i].x << ", " << corners[i].y << ")" << std::endl;
			}

			cv::imshow("Detected Corners with Coordinates", image);
			cv::waitKey(1);
		} else {
			std::cout << "未检测到角点" << std::endl;
		}
	}

	void calib::calibfunc(){
		char cont = 'y';

		while (cont == 'y') {

			std::cout<<"正在标定第"<<n<<"次："<<std::endl;
			std::cout<<"读取相机中的x是:"<<this->cam_x<<std::endl;
			std::cout<<"读取相机中的y是:"<<this->cam_y<<std::endl;
			std::cout<<"请输入机械臂的x坐标:"<<this->arm_x<<std::endl;
			std::cout<<"请输入机械臂的x坐标:"<<this->arm_y<<std::endl;

			// 将新坐标加入列表
			Eigen::Matrix<float, 2, 1> camMatrix, armMatrix;
			camMatrix << cam_x, cam_y;
			armMatrix << arm_x, arm_y;
			coords.emplace_back(camMatrix, armMatrix);

			std::cout << "是否继续标定? (y/n): ";
			std::cin >> cont;
		}
		saveToYAML();
		std::cout << "标定数据已保存到 YAML 文件中。\n";


	}

	void calib::getimg(cv::Mat image) {
		image=this->img;
	}

	void calib::saveToYAML() {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Calibration Data" << YAML::Value << YAML::BeginSeq;

		for (const auto& coord : coords) {
			out << YAML::BeginMap;
			out << YAML::Key << "Camera" << YAML::Value << YAML::Flow << std::vector<float>{coord.first(0), coord.first(1)};
			out << YAML::Key << "Arm" << YAML::Value << YAML::Flow << std::vector<float>{coord.second(0), coord.second(1)};
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		// 写入文件
		std::ofstream fout(filename);
		fout << out.c_str();
		fout.close();

	}

	calib::calib() : cam_x(0), cam_y(0), arm_x(0), arm_y(0), nn(0) {}

} // Transform