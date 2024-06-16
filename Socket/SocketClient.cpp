//
// Created by david on 24-6-13.
//

#include "SocketClient.h"

namespace Networking {

	SocketClient::SocketClient(const std::string& ip, int port)
			: ip(ip), port(port), sock(INVALID_SOCKET), isConnected(false) {
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			std::cerr << "WSAStartup failed: " << result << std::endl;
		}
	}

	SocketClient::~SocketClient() {
		disconnect();
		WSACleanup();
	}

	bool SocketClient::connect() {
		if (isConnected) {
			std::cerr << "[WARNING]: [Socket]: Already connected to a server." << std::endl;
			return false;
		}

		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
			std::cerr << "[ERROR]: [Socket]: Error at socket(): " << WSAGetLastError() << std::endl;
			WSACleanup();
			return false;
		}

		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		inet_pton(AF_INET, ip.c_str(), &clientService.sin_addr);
		clientService.sin_port = htons(port);

		if (::connect(sock, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			closesocket(sock);
			WSACleanup();
			std::cerr << "[ERROR]: [Socket]: Unable to connect to server: " << WSAGetLastError() << std::endl;
			return false;
		}
		std::cout<<"[INFO]: [Socket]: Already connected to PC:"<<ip<<std::endl;
		isConnected = true;
		return true;
	}

	void SocketClient::disconnect() {
		if (isConnected && sock != INVALID_SOCKET) {
			closesocket(sock);
			sock = INVALID_SOCKET;
			isConnected = false;
		}
	}

	bool SocketClient::send(const std::string& message) {
		if (!isConnected) {
			std::cerr << "[ERROR]: [Socket]: Not connected to any server." << std::endl;
			return false;
		}

		int bytesSent = ::send(sock, message.c_str(), message.length(), 0);
		std::cout<<"[INFO]: [Socket]: Successfully send string:"<<message<<std::endl;
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "[ERROR]: [Socket]: Send failed: " << WSAGetLastError() << std::endl;
			disconnect();
			return false;
		}

		return true;
	}
	bool SocketClient::send(const std::vector<uint8_t>& data) {
		if (!isConnected) {
			std::cerr << "[ERROR]: [Socket]: Not connected to any server." << std::endl;
			return false;
		}

		if (data.empty()) {
			std::cerr << "[ERROR]: [Socket]: Data to send is empty." << std::endl;
			return false;
		}

		int bytesSent = ::send(sock, reinterpret_cast<const char*>(data.data()), data.size(), 0);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
			disconnect();
			return false;
		}
		std::cout<<"[INFO]: [Socket]: Successfully send number:";
		for(const auto& elem:data){
			std::cout<<static_cast<int>(elem)<<",";
		}
		std::cout<<std::endl;
		return true;
	}


	std::string SocketClient::receive(int size) {
		if (!isConnected) {
			std::cerr << "Not connected to any server." << std::endl;
			return "";
		}

		char* buffer = new char[size];
		int bytesReceived = recv(sock, buffer, size, 0);
		if (bytesReceived > 0) {
			std::string retStr(buffer, bytesReceived);
			delete[] buffer;
			return retStr;
		} else if (bytesReceived == 0) {
			std::cout << "Connection closed." << std::endl;
		} else {
			std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
		}
		delete[] buffer;
		return "";
	}

} // namespace Networking