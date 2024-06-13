//
// Created by Misaka21 on 24-6-13.
//


//这个.h文件一定要在引用时引用在Windows.h前面，否则会报错
//
#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <string>
#include <memory>
#include <modbus/modbus.h>
#include <iostream>

namespace Networking {
	class ModbusClient {
	public:
		ModbusClient(const std::string& ip, int port);
		~ModbusClient();

		int connect();
		void disconnect();
		/**
		 * @brief 从Modbus服务器读取指定数量的保持寄存器。
		 * 此函数将连接到Modbus服务器，并从指定的开始地址读取多个保持寄存器的值。该操作
		 * 通常用于获取设备的状态或数据。
		 * @param addr 寄存器的起始地址。
		 * @param nb   需要读取的寄存器数量。
		 * @param dest 指向结果数组的指针，用于存储读取的寄存器值。调用者必须确保数组足够大，
		 *             以容纳所有读取的数据。
		 * @throws std::runtime_error 如果连接失败或读取数据时发生错误，将抛出此异常。
		 *         异常消息包含由libmodbus库提供的错误描述。
		 * @example
		 * uint16_t registers[10];
		 * try {
		 *     modbusClient.readRegisters(100, 10, registers);
		 * } catch (const std::runtime_error& e) {
		 *     std::cerr << "Failed to read registers: " << e.what() << std::endl;
		 * }
		 */
		void readRegisters(int addr, int nb, uint16_t *dest);
		/**
		 * @brief 向Modbus服务器写入单个保持寄存器。
		 * 此函数将连接到Modbus服务器，并将一个值写入指定地址的单个保持寄存器。
		 * 该操作通常用于设置设备的配置或控制设备的行为。
		 * @param addr 要写入的寄存器的地址。
		 * @param value 要写入的值。
		 * @throws std::runtime_error 如果连接失败或写入数据时发生错误，将抛出此异常。
		 *         异常消息包含由libmodbus库提供的错误描述。
		 * @example
		 * try {
		 *     modbusClient.writeRegister(100, 12345);
		 * } catch (const std::runtime_error& e) {
		 *     std::cerr << "Failed to write register: " << e.what() << std::endl;
		 * }
		 */
		void writeRegister(int addr, uint16_t value);
		void logInfo(const std::string &message);

	private:
		std::string ip;
		int port;
		modbus_t *ctx;
	};

} // namespace Networking

#endif // MODBUSCLIENT_H
