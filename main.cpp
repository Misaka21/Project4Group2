#include <iostream>
#include "libmodbus/modbus-tcp.h"
#include "libmodbus/modbus.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;
#pragma comment(lib,"ws2_32.lib")
int main() {
	// 初始化 Winsock
	WSADATA wsaData;
	int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsResult != 0) {
		std::cerr << "WSAStartup 失败，错误代码: " << wsResult << std::endl;
		return 1;
	}

	// 创建一个 Modbus TCP 连接
	modbus_t* ctx = modbus_new_tcp("192.168.0.170", 502);
	if (ctx == NULL) {
		std::cerr << "创建 Modbus TCP 上下文失败" << std::endl;
		WSACleanup(); // 清理 Winsock
		return 1;
	}

	// 连接到 Modbus 设备
	if (modbus_connect(ctx) == -1) {
		std::cerr << "连接到 Modbus 设备失败: " << modbus_strerror(errno) << std::endl;
		modbus_free(ctx);
		WSACleanup(); // 清理 Winsock
		return 1;
	}
	else {
		cout << "Connect successfully!" << endl;
	}
	// 设置 Modbus 设备地址
	if (modbus_set_slave(ctx, 1) == -1) {
		std::cerr << "设置 Modbus 设备地址失败: " << modbus_strerror(errno) << std::endl;
		modbus_close(ctx);
		modbus_free(ctx);
		WSACleanup();
		return 1;
	}

	// Sleep(5000);
	// 假设PLC要求接收两个16位整数的坐标数据
	uint16_t points[2] = { 100, 200 };

	// 写入两个寄存器
	int write_result = modbus_write_registers(ctx, 0, 2, points);
	if (write_result == 2) {
		std::cout << "坐标发送成功" << std::endl;
	}
	else {
		std::cerr << "发送坐标失败: " << modbus_strerror(errno) << std::endl;
	}

	// 关闭 Modbus 连接并释放上下文
	//modbus_close(ctx);
	//modbus_free(ctx);

	// 清理 Winsock
	//WSACleanup();

	return 0;
}
