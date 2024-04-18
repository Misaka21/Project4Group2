#ifndef HIK_CAMERA_H
#define HIK_CAMERA_H

#include <stdio.h>
#include <Windows.h>
#include "MvCameraControl.h"
// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

enum CONVERT_TYPE
{
    OpenCV_Mat = 0,
    OpenCV_IplImage = 1,
};

typedef enum GAIN_MODE_
{
    OFF,
    ONCE,
    CONTINUOUS
} GAIN_MODE;

typedef struct CAM_INFO
{
	unsigned int nWidth;//图像宽度
	unsigned int nHeight;//图像高度
	unsigned int nTriggerMode;//触发模式
	unsigned int nTriggerSource;//触发源
	unsigned int nTriggerActivation;//触发极性
	unsigned int nExposureTime;//曝光时间
	unsigned int nGain;//增益
	unsigned int nGamma;//Gamma使能
	unsigned int nPacketSize;//包大小
	unsigned int nHeartBeatTimeout;//心跳超时
	unsigned int nGainMode;//增益模式
	unsigned int nBalanceRatioSelector;//白平衡比例选择
	unsigned int nBalanceRatio;//白平衡比例
	unsigned int nBalanceWhiteAuto;//白平衡白平衡自动
	unsigned int nBalanceWhite;//白平衡白平衡
	unsigned int nBalanceRed;
	unsigned int nBalanceBlue;
	unsigned int nGammaEnable;//Gamma使能
	unsigned int nGammaValue;//Gamma值   
};
void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
class HikCam {

public:
    HikCam();
    ~HikCam();
    void Grab();
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
private:
    int _nRet = MV_OK;
    void* _handle = NULL;
    bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);
};

#endif // HIK_CAMERA_H