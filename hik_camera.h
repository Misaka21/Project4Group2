#ifndef HIK_CAMERA_H
#define HIK_CAMERA_H

#include <cstdio>
#include <iostream>
#include "thread"
#include <atomic>
#include <mutex>
#include "./Includes/MvCameraControl.h"
#include <opencv2/core/core_c.h>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "../debug.h"

#define RED_START "\033[31m"
#define GREEN_START "\033[32m"
#define COLOR_END "\033[0m"

enum CONVERT_TYPE
{
    OpenCV_Mat         = 0,
    OpenCV_IplImage    = 1,
};

typedef enum GAIN_MODE_
{
    OFF ,
    ONCE ,
    CONTINUOUS
} GAIN_MODE;

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
class HikCam {
private:
    int nRet = MV_OK;
    void* handle = NULL;
    std::thread captureThread;
    std::atomic_bool stopCapture;

    //像素排列由RGB转为BGR
    static void RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);

    //回调取流函数
    static unsigned int WorkThread(void* pUser, std::atomic_bool& stop);

public:
    // Constructor and destructor (if needed)
    HikCam();
    ~HikCam();

    //输出相机(没啥卵用)
    bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);

    //void PrintImageInfo(_MV_FRAME_OUT_INFO_EX_ FrameInfo);
    //海康相机的初始化
    bool HikInit(int cam = 0);
    //取图函数封装
    cv::Mat  HikCapture();
    //关闭设备
    int CloseCam();

    /*实验性功能*/
    //设置曝光时间
    bool SetExposureTime(float ExposureTime);
    //设置曝光增益
    bool SetGAIN(int value, float ExpGain);
    //自动白平衡
    bool SetAutoBALANCE();
    //手动白平衡
    bool SetBALANCE(int value, unsigned int value_number);
    //Gamma校正
    bool SetGamma(bool set_status, unsigned int dGammaParam);

//帧数据转换为Mat格式
static bool Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);
};

#endif // HIK_CAMERA_H
