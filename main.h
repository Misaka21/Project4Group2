#pragma once

#include <iostream>
#include <Windows.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core_c.h>

#include"HikCam/hik_camera.h"
#include "Detector/Detector.h"

ImgProcess::Detector::LightParams lightParams={
        0,0.1,15000,20000
};
ImgProcess::Detector::ArmorParams armorParams={
        0,0,500
};




