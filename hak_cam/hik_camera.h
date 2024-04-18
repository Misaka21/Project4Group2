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

#define YELLOW_START "\033[33m"
#define RED_START "\033[31m"
#define GREEN_START "\033[32m"
#define COLOR_END "\033[0m"


enum CONVERT_TYPE
{
    OpenCV_Mat = 0,
    OpenCV_IplImage = 1,
};

typedef enum TRIGGERSOURCE
{
    SOFTWARE,
    LINE0,
    LINE2,
} ;

class CAM_INFO {
public:
    // 设置器方法允许链式调用
    CAM_INFO& setCamID(int id) { _nCamID = id; return *this; }
    CAM_INFO& setWidth(int width) { _nWidth = width; return *this; }
    CAM_INFO& setHeight(int height) { _nHeight = height; return *this; }
    CAM_INFO& setOffsetX(int offsetX) { _nOffsetX = offsetX; return *this; }
    CAM_INFO& setOffsetY(int offsetY) { _nOffsetY = offsetY; return *this; }
    CAM_INFO& setExpTime(float expTime) { _nExpTime = expTime; return *this; }
    CAM_INFO& setGain(float gain) { _nGain = gain; return *this; }
    CAM_INFO& setTrigger(TRIGGERSOURCE trg) { _trigger = trg; return *this; }
    friend class HikCam;  

private:
    int _nCamID = -1;
    int _nWidth = -1;
    int _nHeight = -1;
    int _nOffsetX = -1;
    int _nOffsetY = -1;
    float _nExpTime = -1;
    float _nGain = -1;
    TRIGGERSOURCE _trigger = SOFTWARE;

};
void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
class HikCam {
public:
    HikCam(CAM_INFO Info);
    ~HikCam();
    void Grab();
private:
    int _nRet = MV_OK;
    void* _handle = NULL;
    bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);
    void SetAttribute(CAM_INFO Info);

};

#endif // HIK_CAMERA_H