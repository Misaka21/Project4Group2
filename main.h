#pragma once

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core_c.h>
#include "Modbus/ModbusClient.h"
#include "Socket/SocketClient.h"
#include"HikCam/hik_camera.h"
#include "Detector/Detector.h"
#define YAML_CPP_STATIC_DEFINE
ImgProcess::InsideDetector::PairParams lightParams={
        0,0.1,15000,20000
};
ImgProcess::InsideDetector::InsideBoxParams armorParams={
        0,0,500
};




