#include "hik_camera.h"


bool HikCam::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}



HikCam::HikCam() {

    // ch:初始化SDK | en:Initialize SDK
    _nRet = MV_CC_Initialize();
    if (MV_OK != _nRet)
    {
        printf("Initialize SDK fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:枚举设备 | Enum device
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    _nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != _nRet)
    {
        printf("Enum Devices fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    if (stDeviceList.nDeviceNum > 0)
    {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
        {
            printf("[device %d]:\n", i);
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            if (NULL == pDeviceInfo)
            {
                //break;
            }
            PrintDeviceInfo(pDeviceInfo);
        }
    }
    else
    {
        printf("Find No Devices!\n");
        //break;
    }

    printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
    unsigned int nIndex = 0;


    // ch:选择设备并创建句柄 | Select device and create handle
    _nRet = MV_CC_CreateHandle(&_handle, stDeviceList.pDeviceInfo[nIndex]);
    if (MV_OK != _nRet)
    {
        printf("Create _handle fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:打开设备 | Open device
    _nRet = MV_CC_OpenDevice(_handle);
    if (MV_OK != _nRet)
    {
        printf("Open Device fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
    if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        int nPacketSize = MV_CC_GetOptimalPacketSize(_handle);
        if (nPacketSize > 0)
        {
            _nRet = MV_CC_SetIntValue(_handle, "GevSCPSPacketSize", nPacketSize);
            if (_nRet != MV_OK)
            {
                printf("Warning: Set Packet Size fail _nRet [0x%x]!", _nRet);
            }
        }
        else
        {
            printf("Warning: Get Packet Size fail _nRet [0x%x]!", nPacketSize);
        }
    }
    //_nRet = MV_CC_SetIntValue(_handle, "GevHeartbeatTimeout", 3000);

    // ch:设置触发模式为on | eb:Set trigger mode as on
    _nRet = MV_CC_SetEnumValue(_handle, "TriggerMode", MV_TRIGGER_MODE_ON);
    if (MV_OK != _nRet)
    {
        printf("Set Trigger Mode fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:设置软触发模式 | en:Set Trigger Mode and Set Trigger Source
    _nRet = MV_CC_SetEnumValueByString(_handle, "TriggerSource", "Software");
    if (MV_OK != _nRet)
    {
        printf("Set Trigger Source fail! nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:注册抓图回调 | en:Register image callback
    _nRet = MV_CC_RegisterImageCallBackEx(_handle, ImageCallBackEx, _handle);
    if (MV_OK != _nRet)
    {
        printf("Register Image CallBack fail! _nRet [0x%x]\n", _nRet);
        //break;
    }
    // ch:开始取流 | en:Start grab image
    _nRet = MV_CC_StartGrabbing(_handle);
    if (MV_OK != _nRet)
    {
        printf("Start Grabbing fail! _nRet [0x%x]\n", _nRet);
        //break;
    }
}
void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    //为了等MV_CC_GetImageBuffer调用后再发送软触发命令
    Sleep(30);
    cv::Mat srcImage;
    if (pFrameInfo)
    {
        printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
            pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
    }
    std::cout << "Hello World!\n";
    srcImage = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
    cv::imshow("a", srcImage);
    char key = cv::waitKey(1);
    

}
void HikCam::Grab() {

    _nRet = MV_CC_SetCommandValue(_handle, "TriggerSoftware");
    if (MV_OK != _nRet)
    {
        printf("Send Trigger Software command fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    Sleep(500);
    //如果帧率过小或TriggerDelay很大，可能会出现软触发命令没有全部起效而导致取不到数据的情况

}
HikCam::~HikCam() {
    // ch:停止取流 | en:Stop grab image
    _nRet = MV_CC_StopGrabbing(_handle);
    if (MV_OK != _nRet)
    {
        printf("Stop Grabbing fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:注销抓图回调 | en:Unregister image callback
    _nRet = MV_CC_RegisterImageCallBackEx(_handle, NULL, NULL);
    if (MV_OK != _nRet)
    {
        printf("Unregister Image CallBack fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:关闭设备 | en:Close device
    _nRet = MV_CC_CloseDevice(_handle);
    if (MV_OK != _nRet)
    {
        printf("Close Device fail! _nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:销毁句柄 | en:Destroy handle
    _nRet = MV_CC_DestroyHandle(_handle);
    if (MV_OK != _nRet)
    {
        printf("Destroy _handle fail! _nRet [0x%x]\n", _nRet);
        //break;
    }
    _handle = NULL;


    if (_handle != NULL)
    {
        MV_CC_DestroyHandle(_handle);
        _handle = NULL;
    }


    // ch:反初始化SDK | en:Finalize SDK
    MV_CC_Finalize();
}
