// Group2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include"main.h"


void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	//为了等MV_CC_GetImageBuffer调用后再发送软触发命令
	Sleep(30);
	cv::Mat srcImage;
	if (pFrameInfo)
	{
		printf("[INFO]: Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
	}
	srcImage = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
	/*******从这里开始写代码********/
    ImgProcess::Detector detector_(100,lightParams,armorParams);
    auto Pairs=detector_.detect(srcImage);
    std::cout<<Pairs[0]<<std::endl;

    // 显示结果图像
    cv::imshow("Min Area Rectangles", srcImage);




	/*******从这里开始建议不要动********/
    cv::namedWindow("a", cv::WINDOW_NORMAL);
    cv::resizeWindow("a", 720, 540);
    cv::imshow("a", srcImage);

    char key = cv::waitKey(1);
}
int main()
{
	CAM_INFO camInfo;
	camInfo.setCamID(0)//设置相机ID
		.setWidth(1920)//设置图像宽度
		.setHeight(1080)//设置图像高度
		.setOffsetX(100)//设置图像X偏移
		.setOffsetY(100)//设置图像Y偏移
		.setExpTime(5000)//设置曝光时间
		.setGain(10)//设置增益
		.setTrigger(SOFTWARE);//设置触发方式
	HikCam cam(camInfo);

	while(1)
	cam.Grab();

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
