//
// Created by david on 24-6-16.
//

#include "HandEyeTransform.h"

namespace Transform {


	void Calib::detectboard(){
		// 检测棋盘格角点
		cv::Mat grayImage=this->img.clone(),image=this->img.clone();
		std::vector<cv::Point2f> corners;  // 存储角点
		int board_width = 3;               // 标定板宽度角点数
		int board_height = 3;              // 标定板高度角点数
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
			cam_x=corners[4].x;
			cam_y=corners[4].y;
			cv::namedWindow("Detected Corners with Coordinates", cv::WINDOW_NORMAL);
			cv::resizeWindow("Detected Corners with Coordinates", 1518, 2012);
			cv::imshow("Detected Corners with Coordinates", image);
			cv::waitKey(1);
		} else {
			std::cout << "未检测到角点" << std::endl;
		}
	}

	void Calib::calibfunc(){
		char cont = 'y';

		while (cont == 'y') {
			//std::cout<<"正在标定第"<<n<<"个"<<std::endl;
			std::cout<<"This is NO."<<n++<<"\n";
			detectboard();
			std::cout<<"Read the x-coordinate in the camera:"<<cam_x<<std::endl;
			std::cout<<"Read the y-coordinate in the camera:"<<cam_y<<std::endl;
			std::cout<<"Please enter the x-coordinate of the robotic arm:";
			std::cin>>arm_x;
			std::cout<<"Please enter the y-coordinate of the robotic arm:";
			std::cin>>arm_y;

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
		exit (0);

	}

	void Calib::saveToYAML() {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Calibration Data" << YAML::Value << YAML::BeginSeq;
		int id=1;
		for (const auto& coord : coords) {
			out << YAML::BeginMap;
			out << YAML::Key << "id" <<YAML::Value <<id++;
			out << YAML::Key << "Camera" << YAML::Value << YAML::Flow << std::vector<float>{coord.first(0), coord.first(1)};
			out << YAML::Key << "Arm" << YAML::Value << YAML::Flow << std::vector<float>{coord.second(0), coord.second(1)};
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		// 写入文件
		const std::string& filename="calibration.yaml";
		std::ofstream fout(filename);
		fout << out.c_str();
		fout.close();

	}



	Calib::Calib() : cam_x(0), cam_y(0), arm_x(0), arm_y(0), n(0) {}

	void Calib::linear_regression(const std::vector<std::pair<Eigen::Matrix<float, 2, 1>, Eigen::Matrix<float, 2, 1>>>& tmpcoord,
	                              Eigen::Vector2f& camera_fit_params,
	                              Eigen::Vector2f& arm_fit_params) {
		int n1 = tmpcoord.size();
		Eigen::MatrixXf A(n1, 2);
		Eigen::VectorXf b_camera(n1), b_arm(n);

		for (int i = 0; i < n1; ++i) {
			A(i, 0) = tmpcoord[i].first(0); // Camera x
			A(i, 1) = 1.0;
			b_camera(i) = tmpcoord[i].first(1); // Camera y
			b_arm(i) = tmpcoord[i].second(1);   // Arm y
		}

		// Least squares solution
		camera_fit_params = A.colPivHouseholderQr().solve(b_camera);
		arm_fit_params = A.colPivHouseholderQr().solve(b_arm);
	}

} // Transform