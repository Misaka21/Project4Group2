/*************************************************************************
 * Copyright (c) 2024, Misaka21
 * All rights reserved.
 *
 *
 * File name    :   hik_camera.cpp
 * Brief        :   海康相机的相关函数
 * Revision     :   V3.0
 * Author       :   Misaka21
 * Date         :   2024.04.10
 * Update       :   2023.10.16  V1.0    完成基本代码的编写
 *                  2023.10.18  V1.1    完成单线程的基本代码编写
 *                  2023.10.21  V2.0    完成相机双线程读写缓存
 *                  2023.11.10  V2.1    完成BayerRG转Mat的操作
 *                  2024.01.01  V2.2    完成相机错误代码的优化
 *                  2024.04.20  V3.0    完成产线上工业相机的硬触发和相关参数的设置
 *                  2024.04.28  V3.1    加入Gamma选择和超时时间
 *                  TODO：加入垂直翻转，水平翻转，相机参数输出，简化设置相机参数流程
*************************************************************************/
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



HikCam::HikCam(CAM_INFO Info) {

    // ch:初始化SDK | en:Initialize SDK
    _nRet = MV_CC_Initialize();
    if (MV_OK != _nRet)
    {
        //printf("Initialize SDK fail! nRet [0x%x]\n", _nRet);
        std::cout<<RED_START<<"[ERROR]: Initialize SDK fail! nRet [0x%x]\n"<<COLOR_END<<_nRet<<std::endl;
        //break;
    }

    // ch:枚举设备 | Enum device
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    _nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != _nRet)
    {
        std::cout<<RED_START<<"[ERROR]Enum Devices fail! nRet [0x%x]\n"<<COLOR_END<<_nRet<<std::endl;
        //printf("Enum Devices fail! nRet [0x%x]\n", _nRet);
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
        std::cout<<RED_START<<"[ERROR]: Find No Devices!\n"<<COLOR_END<<std::endl;
        //printf("Find No Devices!\n");
        //break;
    }

    printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
    unsigned int nIndex = Info._nCamID;
    printf(" %d\n", Info._nCamID);


    // ch:选择设备并创建句柄 | Select device and create handle
    _nRet = MV_CC_CreateHandle(&_handle, stDeviceList.pDeviceInfo[nIndex]);
    if (MV_OK != _nRet)
    {
        printf("Create _handle fail! nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:打开设备 | Open device
    _nRet = MV_CC_OpenDevice(_handle);
    if (MV_OK != _nRet)
    {
        printf("%sOpen Device fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Open Device fail! nRet [0x%x]\n", _nRet);
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
                printf("%s[Warning]: Set Packet Size fail nRet [0x%x]!%s\n", YELLOW_START, _nRet, COLOR_END);
                //printf("Warning: Set Packet Size fail nRet [0x%x]!", _nRet);
            }
        }
        else
        {
            printf("%s[Warning]: Get Packet Size fail nRet [0x%x]!%s\n", YELLOW_START, nPacketSize, COLOR_END);
            //printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
        }
    }
    //_nRet = MV_CC_SetIntValue(_handle, "GevHeartbeatTimeout", 3000);

    // ch:设置触发模式为on | eb:Set trigger mode as on
    _nRet = MV_CC_SetEnumValue(_handle, "TriggerMode", MV_TRIGGER_MODE_ON);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Set Trigger Mode fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Set Trigger Mode fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    SetAttribute(Info);


    // ch:注册抓图回调 | en:Register image callback
    _nRet = MV_CC_RegisterImageCallBackEx(_handle, ImageCallBackEx, _handle);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Register Image CallBack fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Register Image CallBack fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    // ch:开始取流 | en:Start grab image
    _nRet = MV_CC_StartGrabbing(_handle);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Start Grabbing fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Start Grabbing fail! nRet [0x%x]\n", _nRet);
        //break;
    }
}

void HikCam::Grab() {

    _nRet = MV_CC_SetCommandValue(_handle, "TriggerSoftware");
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Send Trigger Software command fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Send Trigger Software command fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    Sleep(500);
    //如果帧率过小或TriggerDelay很大，可能会出现软触发命令没有全部起效而导致取不到数据的情况
}
void HikCam::SetAttribute(CAM_INFO Info) {
    // 设置触发模式
    if (Info._nTrigger == SOFTWARE)
    {
        _nRet = MV_CC_SetEnumValueByString(_handle, "TriggerSource", "Software");
        if (MV_OK != _nRet)
        {
            printf("%s[WARNING]: Set Trigger Software fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
			//printf("Set Trigger Software fail! nRet [0x%x]\n", _nRet);
			//break;
		}
	}
	if (Info._nTrigger == LINE0)
	{
		_nRet = MV_CC_SetEnumValueByString(_handle, "TriggerSource", "Line0");
        if (MV_OK != _nRet)
        {
            printf("%s[WARNING]: Set Trigger Line0 fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
			//printf("Set Trigger Line0 fail! nRet [0x%x]\n", _nRet);
			//break;
		}
	}
	if (Info._nTrigger == LINE2)
	{
		_nRet = MV_CC_SetEnumValueByString(_handle, "TriggerSource", "Line2");
        if (MV_OK != _nRet)
        {
            printf("%s[WARNING]: Set Trigger Line2 fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
			//printf("Set Trigger Line2 fail! nRet [0x%x]\n", _nRet);
			//break;
		}
    }
    // 设置曝光时间
    _nRet = MV_CC_SetFloatValue(_handle, "ExposureTime", Info._nExpTime);
    if (MV_OK != _nRet)
    {
        printf("%s[WARNING]: Set ExposureTime fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
		//printf("Set ExposureTime fail! nRet [0x%x]\n", _nRet);
		//break;
	}
    // 设置增益
    _nRet = MV_CC_SetFloatValue(_handle, "Gain", Info._nGain);
    if (MV_OK != _nRet)
    {
        printf("%s[WARNING]: Set Gain fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
        //printf("Set Gain fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    // 设置宽度
    _nRet = MV_CC_SetIntValue(_handle, "Width", Info._nWidth);
    if (MV_OK != _nRet)
    {
        printf("%s[WARNING]: Set Width fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
		//printf("Set Width fail! nRet [0x%x]\n", _nRet);
		//break;
	}
    // 设置高度
	_nRet = MV_CC_SetIntValue(_handle, "Height", Info._nHeight);
    if (MV_OK != _nRet)
    {
        printf("%s[WARNING]: Set Height fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
        // printf("Set Height fail! nRet [0x%x]\n", _nRet);
		//break;
	}
	// 设置偏移X
	_nRet = MV_CC_SetIntValue(_handle, "OffsetX", Info._nOffsetX);
    if (MV_OK != _nRet)
    {
        printf("%s[WARNING]: Set OffsetX fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
		//printf("Set OffsetX fail! nRet [0x%x]\n", _nRet);
		//break;
	}
	// 设置偏移Y
	_nRet = MV_CC_SetIntValue(_handle, "OffsetY", Info._nOffsetY);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Set OffsetY fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
		//printf("Set OffsetY fail! nRet [0x%x]\n", _nRet);
		//break;
	}
    //设置心跳时间
    _nRet = MV_CC_SetIntValue(_handle, "GevHeartbeatTimeout", Info._nHeartTimeOut);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Set HeartbeatTimeout fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
        //printf("Set OffsetY fail! nRet [0x%x]\n", _nRet);
        //break;
    }
    //设置Gamma使能
    _nRet = MV_CC_SetBoolValue(_handle, "GammaEnable", Info._nGamma);
    if(Info._nGamma)
        _nRet = MV_CC_SetEnumValue(_handle, "GammaSelector", Info._nGamma);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Set GammaMode fail! nRet [0x%x]%s\n", YELLOW_START, _nRet, COLOR_END);
    }

    //输出当前设置
    printf("%s", GREEN_START);
    printf("Current Setting:\n");
    printf("ExposureTime: %f\n", Info._nExpTime);
    printf("Gain: %f\n", Info._nGain);
    printf("Width: %d\n", Info._nWidth);
    printf("Height: %d\n", Info._nHeight);
    printf("HeartbeatTimeout: %d\n", Info._nHeartTimeOut);
    printf("OffsetX: %d\n", Info._nOffsetX);
    printf("OffsetY: %d\n", Info._nOffsetY);
    auto getTriggerSource = [](TRIGGERSOURCE trigger) -> const char* {
        switch (trigger) {
        case SOFTWARE: return "SOFTWARE";
        case LINE0:    return "LINE0";
        case LINE2:    return "LINE2";
        default:       return"ERROR";
        }
    };
    printf("TriggerSource: %s\n", getTriggerSource(Info._nTrigger));
    auto getGammaMode = [](GAMMAMODE nGamma) -> const char* {
        switch (nGamma) {
            case OFF:       return "OFF";
            case USER:      return "User";
            case sRGB:      return "sRGB";
            default:        return "ERROR";
        }
    };
    printf("GammaMode: %s\n", getGammaMode(Info._nGamma));
    printf("%s", COLOR_END);
    
    

}
HikCam::~HikCam() {
    // ch:停止取流 | en:Stop grab image
    _nRet = MV_CC_StopGrabbing(_handle);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Stop Grabbing fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Stop Grabbing fail! nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:注销抓图回调 | en:Unregister image callback
    _nRet = MV_CC_RegisterImageCallBackEx(_handle, NULL, NULL);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Unregister Image CallBack fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Unregister Image CallBack fail! nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:关闭设备 | en:Close device
    _nRet = MV_CC_CloseDevice(_handle);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Close Device fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Close Device fail! nRet [0x%x]\n", _nRet);
        //break;
    }

    // ch:销毁句柄 | en:Destroy handle
    _nRet = MV_CC_DestroyHandle(_handle);
    if (MV_OK != _nRet)
    {
        printf("%s[ERROR]: Destroy _handle fail! nRet [0x%x]%s\n", RED_START, _nRet, COLOR_END);
        //printf("Destroy _handle fail! nRet [0x%x]\n", _nRet);
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
