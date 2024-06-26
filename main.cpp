#include"main.h"
#define DEFAULT_PORT "465"


Networking::SocketClient sock2pc("172.16.24.157",465);
Networking::ModbusClient mod2plc("172.16.26.170", 504,20);
Transform::Calib calib;

void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	//为了等MV_CC_GetImageBuffer调用后再发送软触发命令
	//Sleep(30);
	cv::Mat srcImage;
	if (pFrameInfo)
	{
		printf("[INFO]: Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
	}
	srcImage = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
	/*******从这里开始写代码********/

	//cv::transpose(srcImage, srcImage);
	//cv::flip(srcImage, srcImage, 1);
	calib.img=srcImage.clone();
	cv::Mat Image=srcImage.clone();

    ImgProcess::InsideDetector detector_(230,insideParams,outsideParams);
    auto Pairs=detector_.detect(Image);
	ImgProcess::OutsideDetector outsidedbox;
	auto Obox_list=outsidedbox.outsideprocess(Image);
	if(Pairs.size()==5){
		try{
			mod2plc.writeRegister (0,1);

			std::cout<<Pairs[2].center.x<<" "<<Pairs[2].center.y<<" "<<Pairs[2].center.x*0.19+468.83<<" "<<Pairs[2].center.y*-0.19+580.91<<std::endl;
			mod2plc.writeRegister (1,Networking::swapHighBite ((uint16_t )(Pairs[2].center.x*0.19+468.83)));
			mod2plc.writeRegister (2,Networking::swapHighBite ((uint16_t )(Pairs[2].center.y*-0.19+580.91)));


			//mod2plc.writeRegister (3,Networking::swapHighBite ((uint16_t )(Pairs[0].center.x*0.19+468.83)));
			//mod2plc.writeRegister (4,Networking::swapHighBite ((uint16_t )(Pairs[0].center.y*-0.19+580.91)));
//			mod2plc.writeRegister (0,1);
//			mod2plc.writeRegister (1,250);
//			mod2plc.writeRegister (2,251);
//			mod2plc.writeRegister (3,257);
		}catch(const std::runtime_error &e){
			std::cerr<<e.what()<<std::endl;
		}
	}


    //std::cout<<Pairs[0]<<std::endl;

     //显示结果图像
    cv::imshow("Min Area Rectangles", srcImage);




	/*******从这里开始建议不要动********/
    cv::namedWindow("a", cv::WINDOW_NORMAL);
    cv::resizeWindow("a", 1000, 1000);
    cv::imshow("a", srcImage);

    char key = cv::waitKey(1);
}

//#pragma comment(lib,"ws2_32.lib")
int main() {
	mod2plc.connect();
	//sock2pc.connect ();
	//sock2pc.connect();
	//sock2pc.send ("fuck");

	//sock2pc.receive(1);
	//sock2pc.send ("20240404");


	//std::vector <uint8_t> sentt={2,0,2,4,0,4,0,4};
	//sock2pc.send (sentt);
	//mod2plc.writeRegister (0,1);

	//std::cout<<Pairs[2].center.x<<" "<<Pairs[2].center.y<<" "<<Pairs[2].center.x*0.19+524.81<<" "<<Pairs[2].center.y*-0.19+586.67<<std::endl;
//	mod2plc.writeRegister (1,Networking::swapHighBite ((uint16_t )(Pairs[0].center.x*0.19+524.81)));
//	mod2plc.writeRegister (2,Networking::swapHighBite ((uint16_t )(Pairs[0].center.y*-0.19+586.67)));


//
	//mod2plc.writeRegister (2,255);

	CAM_INFO camInfo;
	camInfo.setCamID (0)//设置相机ID
			.setWidth (4024)//设置图像宽度
			.setHeight (3036)//设置图像高度
			.setOffsetX (0)//设置图像X偏移
			.setOffsetY (0)//设置图像Y偏移
			.setExpTime (8000)//设置曝光时间
			.setGain (18)//设置增益
			.setHeartTimeOut (500)//设置超时时间
			.setTrigger (SOFTWARE)//设置触发方式
			.setGamma (GAMMA_OFF);//设置Gamma模式
	HikCam cam (camInfo);
//
	while (1) {
		//mod2plc.writeRegister (2,255);
		cam.Grab ();
		//calib.calibfunc ();
		//std::cout<<calib.cam_x<<" "<<calib.cam_y<<" "<<calib.cam_x*-0.18+1151.04<<" "<<calib.cam_y*0.18+35.24<<std::endl;


		//Sleep (300000);
	}
	return 0;
}

/*
int main()
{

	CAM_INFO camInfo;
    camInfo.setCamID(0)//设置相机ID
            //.setWidth(1920)//设置图像宽度
            //.setHeight(1080)//设置图像高度
            .setOffsetX(0)//设置图像X偏移
            .setOffsetY(0)//设置图像Y偏移
            .setExpTime(5000)//设置曝光时间
            .setGain(10)//设置增益
            .setHeartTimeOut(5000)//设置超时时间
            .setTrigger(SOFTWARE)//设置触发方式
            .setGamma(sRGB);//设置Gamma模式
    HikCam cam(camInfo);

	while(1)
	cam.Grab();

}
*/
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
