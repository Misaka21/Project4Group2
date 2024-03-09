/*************************************************************************
 * Copyright (c) 2023, Misaka21
 * All rights reserved.
 *
 *
 * File name    :   hik_camera.cpp
 * Brief        :   海康相机的相关函数
 * Revision     :   V2.0
 * Author       :   Misaka21
 * Date         :   2023.10.16
 * Update       :   2023.10.16  V1.0    完成基本代码的编写
 *                  2023.10.18  V1.1    完成单线程的基本代码编写
 *                  2023.10.21  V2.0    完成相机双线程读写缓存
 *                  2023.11.10  V2.1    完成BayerRG转Mat的操作
 *                  2024.01.01  V2.2    完成相机错误代码的优化
 *                  TODO：加入垂直翻转，水平翻转，相机参数输出，简化设置相机参数流程
*************************************************************************/
#include <thread>
#include "hik_camera.h"
static cv::Mat srcImageA,srcImageB;
std::atomic_bool bufferAFull;
std::atomic_bool bufferBFull;
std::mutex mutexBufferA, mutexBufferB;
std::atomic_bool bufferRefreshA;

/******************************************************
 * Brief     :海康相机的初始化
 * Author    :Misaka21
 * Parameter :
 *      int CAM: 有多个相机时选择第几个相机，默认为0
 * Return    : 无.
*******************************************************/
bool HikCam::HikInit(int cam){
	// ch:枚举设备 | en:Enum device
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return false;
	}

	if (stDeviceList.nDeviceNum > 0)
	{
		for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
		{
			printf("[device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
			if (NULL == pDeviceInfo)
			{
				break;
			}
			PrintDeviceInfo(pDeviceInfo);
		}
	}
	else
	{
		printf("Find No Devices!\n");
		exit(0);
		return false;
	}

	printf("Please Input camera index(0-%d):\n", stDeviceList.nDeviceNum-1);
	unsigned int nIndex = cam;
	//scanf_s("%d", &nIndex);

	if (nIndex >= stDeviceList.nDeviceNum)
	{
		printf("Input error!\n");
		return false;
	}

	// ch:选择设备并创建句柄 | en:Select device and create handle
	nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
		return false;
	}

	// ch:打开设备 | en:Open device
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		printf("%s[ERROR]设备被占用，nRet [0x%x]\n%s", RED_START,nRet,COLOR_END);
		exit(0);
		return false;
	}

	// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
	if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
		if (nPacketSize > 0)
		{
			nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
			if(nRet != MV_OK)
			{
				printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
			}
		}
		else
		{
			printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
		}
	}


	// ch:设置触发模式为off | en:Set trigger mode as off
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		return false;
	}
	// Set number of image node
	//unsigned int nImageNodeNum = 2;
	//nRet = MV_CC_SetImageNodeNum(handle, nImageNodeNum);
	//if (MV_OK != nRet)
	//{
	//    printf("Set number of image node fail! nRet [0x%x]\n", nRet);
	//    return false;
	//}

	// 调节亮度对比度白平衡
/**************此处添加调节相机参数代码*****************/
	//SetAutoBALANCE();
	SetExposureTime(8000);
	//SetGAIN(2,1);
/***********************************/

	// ch:开始取流 | en:Start grab image
	nRet = MV_CC_StartGrabbing(handle);

	if (MV_OK != nRet)
	{
		printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
		return false;
	}

	return true;
}

// ch:等待按键输入 | en:Wait for key press
//void WaitForKeyPress(void)
//{
//    while(!_kbhit())
//    {
//        Sleep(10);
//    }
//    _getch();
//}

//输出相机参数
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
		printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
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

/******************************************************
 * Brief     :回调取流函数
 * Author    :Misaka21
 * Parameter :
 *      void* pUser: 相机句柄
 *      Mat* pMat  : 传递给Convert2Mat的图片指针
 * Return    : 无.
*******************************************************/
unsigned int HikCam::WorkThread(void* pUser, std::atomic_bool& stop)//static  unsigned int WorkThread(void* pUser,cv::Mat* pMat)
{

	bool bConvertRet;
	int nRet_tmp = MV_OK;
	MV_FRAME_OUT stOutFrame = {0};

	do{
		nRet_tmp = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000);
		if (nRet_tmp == MV_OK)
		{
			//printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n",
			//stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

			nRet_tmp = MV_CC_FreeImageBuffer(pUser, &stOutFrame);
			bConvertRet = Convert2Mat(&stOutFrame.stFrameInfo, stOutFrame.pBufAddr);
			if (!bConvertRet)   printf("%s[ERROR]不支持的图像格式\n%s",RED_START,COLOR_END);
			if(nRet_tmp != MV_OK)
			{
				printf("Free Image Buffer fail! nRet [0x%x]\n", nRet_tmp);
			}
		}
		else
		{
			printf("%s[ERROR]Get Image fail! nRet [0x%x]%s\n", RED_START,nRet_tmp,COLOR_END);
			//exit(0);
		}
		//if(g_bExit) break;

	}while(!stop.load());
	return 0;
}

// ch:像素排列由RGB转为BGR | en:Convert pixel arrangement from RGB to BGR
void HikCam::RGB2BGR( unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight )
{
	if ( NULL == pRgbData )
	{
		return;
	}

	// red和blue数据互换
	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3]     = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}
}

/******************************************************
 * Brief     :帧数据转换为Mat格式图片并传回主函数
 * Author    :Misaka21
 * Parameter :
 *      MV_FRAME_OUT_INFO_EX* pstImageInfo: 输出帧信息结构体
 *      Mat* pMat  : 传递给主函数的图片指针
 * Return    : 无.
*******************************************************/
bool HikCam::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData)
{
	if (NULL == pstImageInfo || NULL == pData)
	{
		printf("NULL info or data.\n");
		return false;
	}

	cv::Mat srcImage;
	if ( PixelType_Gvsp_Mono8 == pstImageInfo->enPixelType )                // Mono8类型
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else if ( PixelType_Gvsp_RGB8_Packed == pstImageInfo->enPixelType )     // RGB8类型
	{
		// Mat像素排列格式为BGR，需要转换
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	else if(PixelType_Gvsp_BayerRG8 == pstImageInfo->enPixelType)     // bayerRGB8类型
	{
		/* Bayer 格式转换mat格式的方法:
		1. 使用相机句柄销毁前 调用 MV_CC_ConvertPixelType 将PixelType_Gvsp_BayerRG8 等Bayer格式转换成 PixelType_Gvsp_BGR8_Packed
		2. 参考上面 将BGR转换为 mat格式
		*/
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
		cv::cvtColor(srcImage,srcImage,cv::COLOR_BayerRG2RGB);
	
		//printf("Unsupported pixel format\n");
		//return false;
	} else {printf("%s[ERROR]不支持的图片格式\n%s",RED_START,COLOR_END);exit(0);}

	if ( NULL == srcImage.data )
	{
		printf("Creat Mat failed.\n");
		exit(0);
		return false;
	}

	try
	{
		// ch:保存Mat图片 | en:Save converted image in a local file
		//cv::imwrite("Image_Mat.bmp", srcImage);
		//cv::imshow("show",srcImage);
		//cv::waitKey(1);
		if (bufferAFull == false)
		{
			mutexBufferA.lock();
			srcImageA = srcImage.clone();
			bufferRefreshA = false;
			bufferAFull = true;
			mutexBufferA.unlock();
		}
		else
		{
			if (bufferBFull == false)
			{
				mutexBufferB.lock();
				srcImageB = srcImage.clone();
				bufferRefreshA = true;
				bufferBFull = true;
				mutexBufferB.unlock();
			}
			else
			{
				if (bufferRefreshA)
				{
					bufferAFull = false;
					mutexBufferA.lock();
					srcImageA = srcImage.clone();
					bufferRefreshA = false;
					bufferAFull = true;
					mutexBufferA.unlock();
				}
				else
				{
					bufferBFull = false;
					mutexBufferB.lock();
					srcImageB = srcImage.clone();
					bufferRefreshA = true;
					bufferBFull = true;
					mutexBufferB.unlock();
				}
			}
		}
	}
	catch (cv::Exception& ex)
	{
		fprintf(stderr, "Exception in saving mat image: %s\n", ex.what());
	}


	srcImage.release();

	return true;
}


/******************************************************
 * Brief     :封装调用函数，每次取一张图
 * Author    :Misaka21
 * Parameter :
 *      Mat* pMat  : Mat指针，表示要更改的图片地址
 * Return    :返回Mat矩阵（图片）
*******************************************************/
cv::Mat HikCam::HikCapture() {
	cv::Mat srcImage;
	while (true) { //无限循环直到获取到数据
		if (bufferAFull == true)
		{
			mutexBufferA.lock();
			srcImage = srcImageA;
			bufferAFull = false;
			mutexBufferA.unlock();
			return srcImage;  // 成功获取到图像
		}
		else if (bufferBFull == true)
		{
			mutexBufferB.lock();
			srcImage = srcImageB;
			bufferBFull = false;
			mutexBufferB.unlock();
			return srcImage;  // 成功获取到图像
		}
	}
}


// ch:关闭设备 | Close device
int HikCam::CloseCam(){
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
	{
		printf("ClosDevice fail! nRet [0x%x]\n", nRet);
		return 0;
	}

	// ch:销毁句柄 | Destroy handle
	nRet = MV_CC_DestroyHandle(handle);
	if (MV_OK != nRet)
	{
		printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
		return 0;
	}
	if (nRet != MV_OK)
	{
		if (handle != NULL)
		{
			MV_CC_DestroyHandle(handle);
			handle = NULL;
		}
	}
	return 0;
}

/******************************************************
 * Brief     :设置曝光时间
 * Author    :Misaka21
 * Parameter :
 *      float ExposureTime  :  Min: 15.000000
 *                             Max: 9999723.000000
 * Return    : 无.
*******************************************************/
bool HikCam::SetExposureTime(float ExposureTime)
{   //设置曝光时间
	nRet = MV_CC_SetFloatValue(handle, "ExposureTime", ExposureTime);
	if(nRet == MV_OK)
	{
		std::cout<<GREEN_START <<"[INFO] 曝光值设置成功"<<COLOR_END <<std::endl;
		return true;
	}
	else
	{
		std::cout<<RED_START <<"[WARNING] 曝光值设置失败"<<COLOR_END <<std::endl;
		return false;
	}
}

/******************************************************
 * Brief     :设置增益
 * Author    :Misaka21
 * Parameter :
 *      int value           :  0:自动增益关闭
 *                             1:自动增益一次
 *                             2:连续自动增益
 *                             其他:设置固定增益
 *      float ExpGain       :  固定增益的数值,满足以下:
 *                             Min: 0.000000
 *                             Max: 17.016600
 * Return    : 无.
*******************************************************/
bool HikCam::SetGAIN(int value, float ExpGain)
{   //曝光增益
	nRet = MV_CC_SetFloatValue(handle, "AutoGainLowerLimit ", 0.000000);
	nRet = MV_CC_SetFloatValue(handle, "AutoGainupperLimit ", 10.016600);
	switch (value) {
		case 0:{nRet = MV_CC_SetEnumValue(handle, "GainAuto", MV_GAIN_MODE_OFF);break;}
		case 1:{nRet = MV_CC_SetEnumValue(handle, "GainAuto", MV_GAIN_MODE_ONCE);break;}
		case 2:{nRet = MV_CC_SetEnumValue(handle, "GainAuto", MV_GAIN_MODE_CONTINUOUS);break;}
		default:
		{
			nRet = MV_CC_SetEnumValue(handle, "GainAuto", OFF);
			nRet = MV_CC_SetFloatValue(handle, "Gain", ExpGain);
			if(nRet == MV_OK)
			{
				std::cout<<GREEN_START <<"[INFO] 设置曝光增益成功！"<<COLOR_END <<std::endl;
				return true;
			}
			else
			{
				std::cout<<RED_START <<"[WARNING] 设置曝光增益失败！"<<COLOR_END <<std::endl;
				return false;
			}
		}
	}

	//nRet = MV_CC_SetFloatValue(handle, "Gain", (float)ExpGain);
	if(nRet == MV_OK)
	{
		std::cout<<GREEN_START <<"[INFO] 设置曝光增益成功！"<<COLOR_END <<std::endl;
		return true;
	}
	else
	{
		std::cout<<RED_START <<"[WARNING] 设置曝光增益失败！"<<COLOR_END <<std::endl;
		return false;
	}
}

/******************************************************
 * Brief     :设置自动白平衡
 * Author    :Misaka21
 * Return    : 无.
*******************************************************/
bool HikCam::SetAutoBALANCE()
{
	nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", 1);
	if(nRet != MV_OK)
	{
		std::cout<<RED_START <<"[WARNING] 自动白平衡设置失败！"<<COLOR_END <<std::endl;
		return false;
	}
	else
	{
		std::cout<<GREEN_START <<"[INFO] 自动白平衡设置成功！"<<COLOR_END <<std::endl;
		return true;
	}
}

/******************************************************
 * Brief     :设置手动白平衡(关闭自动)
 *      int value           :  0:设置白平衡 红
 *                             1:设置白平衡 绿
 *                             2:设置白平衡 蓝色
 *      float value_number  :  固定增益的数值,满足以下:
 *                             Min:  1
 *                             Max:  16376
 * Author    :Misaka21
 * Return    : 无.
*******************************************************/
bool HikCam::SetBALANCE(int value, unsigned int value_number)
{   //手动白平衡（具有记忆功能））
	//关闭自动白平衡
	nRet = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_OFF);
	if(nRet != MV_OK)
	{
		printf("[CAMERA] 关闭自动白平衡失败！\n");
		return false;
	}

	//设置RGB三通道白平衡值
	if(value == 0)
	{
		nRet = MV_CC_SetBalanceRatioRed(handle, value_number);

		if(nRet == MV_OK)
		{
			printf("[CAMERA] set R_Balance succeed！\n");
			return true;
		}
		else
		{
			printf("[CAMERA] set R_Balance failed！\n");
			return false;
		}
	}
	else if(value == 1)
	{
		nRet = MV_CC_SetBalanceRatioGreen(handle, value_number);

		if(nRet == MV_OK)
		{
			printf("[CAMERA] set G_Balance succeed！\n");
		}
		else
		{
			printf("[CAMERA] set G_Balance failed！\n");
			return false;
		}
	}
	else if(value == 2)
	{
		nRet = MV_CC_SetBalanceRatioBlue(handle, value_number);

		if(nRet == MV_OK)
		{
			printf("[CAMERA] set B_Balance succeed！\n");
		}
		else
		{
			printf("[CAMERA] set B_Balance failed！\n");
			return false;
		}
	}
	return true;
}

/******************************************************
 * Brief     :设置手动白平衡(关闭自动)
 *      int value           :  0:设置白平衡 红
 *                             1:设置白平衡 绿
 *                             2:设置白平衡 蓝色
 *      float value_number  :  固定增益的数值,满足以下:
 *                             Min:  1
 *                             Max:  16376
 * Author    :Misaka21
 * Return    : 无.
*******************************************************/
bool HikCam::SetGamma(bool set_status, unsigned int dGammaParam)
{   //设置Gamma值
	if(set_status)
	{
		nRet = MV_CC_GetBoolValue(handle, "GammaEnable", &set_status);
		if(nRet == MV_OK)
		{
			printf("[CAMERA] 设置Gamma值成功！\n");
			return true;
		}
		else
		{
			printf("[CAMERA] 设置Gamma值失败！\n");
			return false;
		}
	}
	else
	{
		nRet = MV_CC_SetEnumValue(handle, "Gamma", 0);
		if(nRet == MV_OK)
		{
			printf("[CAMERA] 关闭Gamma值成功！\n");
			return true;
		}
		else
		{
			printf("[CAMERA] 关闭Gamma值失败！\n");
			return false;
		}
	}
}

HikCam::HikCam(): stopCapture(false){
	HikInit();
	//std::thread captureThread;

	captureThread= std::thread(WorkThread,handle, std::ref(stopCapture));
}

HikCam::~HikCam(){
	CloseCam();
	stopCapture.store(true);
	if (captureThread.joinable()) {
		captureThread.join();
	}
	std::cout<<"bye"<<std::endl;
}
