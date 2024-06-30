//
// Created by david on 24-6-13.
//

#ifndef GROUP2_SOCKETCLIENT_H
#define GROUP2_SOCKETCLIENT_H
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#pragma comment(lib, "Ws2_32.lib")
namespace Networking {

	class SocketClient {
	public:
		SocketClient(const std::string& ip, int port);
		~SocketClient();

		bool connect();
		void disconnect();
		//函数重载,你想发啥发啥
		bool send(const cv::Mat& mat);
		bool send(const std::string& message);
		bool send(const std::vector<uint8_t>& data);

		std::string receive(int size = 1024);

	private:
		std::string ip;
		int port;
		SOCKET sock;
		bool isConnected;
	};

} // namespace Networking

#endif //GROUP2_SOCKETCLIENT_H
