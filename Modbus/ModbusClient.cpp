//
// Created by david on 24-6-13.
//

#include "ModbusClient.h"
#include <stdexcept>

namespace Networking {


	ModbusClient::ModbusClient(const std::string &ip, int port,int slave_id) : ip (ip), port (port), ctx (nullptr),  slave_id(slave_id){
		ctx = modbus_new_tcp (ip.c_str (), port);
		if (ctx == nullptr) {
			std::cerr<<"[ERROR]: Unable to create the libmodbus context"<<std::endl;
		} else {
			logInfo ("Modbus TCP context created");
		}
	}



	ModbusClient::~ModbusClient() {
		if (ctx) {
			modbus_close (ctx);
			modbus_free (ctx);
		}
	}

	int ModbusClient::connect() {
		if (ctx == nullptr) {
			return -1; // Indicate error in context creation
		}
		if (modbus_connect (ctx) == -1) {
			std::cerr<<"[ERROR]: Connection failed: " + std::string (modbus_strerror (errno))<<std::endl;
			return -1; // Connection failed
		}
		logInfo ("Connected to the server");

		if(modbus_set_slave (ctx,slave_id)==-1){
			std::cerr<<"[ERROR]: Set Slave id failed: " + std::string (modbus_strerror (errno))<<std::endl;
			return -1; // Connection failed
		}


		return 0; // Success
	}

	void ModbusClient::disconnect() {
		if (ctx) {
			modbus_close (ctx);
			logInfo ("Connection closed");
		}
	}

	void ModbusClient::readRegisters(int addr, int nb, uint16_t *dest) {
		if (ctx == nullptr) {
			throw std::runtime_error("Modbus context not initialized");
		}
		if (modbus_read_registers(ctx, addr, nb, dest) == -1) {
			//logInfo("[ERROR]: Failed to read registers: " + std::string(modbus_strerror(errno)));
			throw std::runtime_error("[ERROR]: Failed to read registers: " + std::string(modbus_strerror(errno)));
		}
		logInfo("Registers read successfully");
	}

	void ModbusClient::writeRegister(int addr, uint16_t value) {
		if (ctx == nullptr) {
			throw std::runtime_error("Modbus context not initialized");
		}
		if (modbus_write_register(ctx, addr, value) == -1) {
			//logInfo("[ERROR]: Failed to write register: " + std::string(modbus_strerror(errno)));
			throw std::runtime_error("[ERROR]: Failed to write register: " + std::string(modbus_strerror(errno)));
		}
		logInfo("Written to["+std::to_string(addr)+"]="+std::to_string(value)+" Successfully!");
	}

	void ModbusClient::logInfo(const std::string &message) {
		std::cout << "[INFO]: [ModBus]: " << message << "\n";
	}
}