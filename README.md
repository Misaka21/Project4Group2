## 学❤长❤推❤荐
# 闲❤着❤没❤事❤干？
## 来❤调❤产❤线❤吧


本段产线是第二工位的摄像头代码，主要代码架构如下，
```shell
./
.gitattributes      #要匹配的文件
.gitignore          #要隐藏的文件
CMakeLists.txt      #CMake编译文件
calibration.yaml    #标定的文件   
Linear_fit.py       #相机/机械臂坐标转换文件
main.cpp            #主函数
main.h
README.md
├── .git/           #git相关文件
├── build/          #编译相关文件
├── Detector/       #书签盒检测文件
│       box.h
│       Detector.cpp
│       Detector.h
├── HandEye/        #手眼标定封装类
│       HandEyeTransform.cpp
│       HandEyeTransform.h
├── HikCam/         #海康相机调用封装类
│       hik_camera.cpp
│       hik_camera.h
│   ├── Includes/   #海康相机对应头文件
│   │       CameraParams.h
│   │       MvCameraControl.h
│   │       MvErrorDefine.h
│   │       MvISPErrorDefine.h
│   │       MvObsoleteInterfaces.h
│   │       MvSdkExport.h
│   │       ObsoleteCamParams.h
│   │       PixelType.h
├── Includes/       #对应静态库的头文件
│   ├── Eigen/
│   ├── modbus/
│   ├── opencv2/
│   ├── yaml-cpp/
├── Libs/           #对应的静态库
│       modbus.lib
│       modbusd.lib
│       MvCameraControl.lib
│       opencv_world455.lib
│       opencv_world455d.lib
│       yaml-cpp.lib
│       yaml-cppd.lib
├── Modbus/         #ModBus的调用封装类
│       ModbusClient.cpp
│       ModbusClient.h
├── Socket/         #Socket的调用封装类
│       SocketClient.cpp
│       SocketClient.h
```
其中，本代码的一大亮点是将抽象函数高度封装，并有多种错误处理
## Hik_Camera类
CAM_INFO类使用链式调用的方式，允许用户在实例化之前设置相机参数，如ID、分辨率、偏移量、曝光时间、增益、触发源等。这种设计使得设置参数变得更加直观和方便。接着HikCam类封装了相机的初始化、图像抓取等功能。构造函数接收一个CAM_INFO对象，用于初始化相机参数。Grab成员函数用于启动图像抓取过程。
### 调用方法：
```c++
	CAM_INFO camInfo;
	camInfo.setCamID(0)//设置相机ID
			.setWidth(4024)//设置图像宽度
			.setHeight(3036)//设置图像高度
			.setOffsetX(0)//设置图像X偏移
			.setOffsetY(0)//设置图像Y偏移
			.setExpTime(6000)//设置曝光时间
			.setGain(18)//设置增益
			.setHeartTimeOut(500)//设置超时时间
			.setTrigger(SOFTWARE)//设置触发方式
			.setGamma(GAMMA_OFF);//设置Gamma模式
	HikCam cam(camInfo);
```
## SocketClient类
SocketClient是Networking命名空间下的类，用于实现基于TCP/IP协议的网络客户端通信，给上位机发送socket指令。SocketClient类接受IP地址和端口号作为参数，在构造函数中初始化这些值。

数据在发送时send函数被重载，支持发送cv::Mat、std::string和std::vector<uint8_t>三种类型的数据，增强了通用性。

### 调用方法：
```c++
Networking::SocketClient sock2pc("172.16.24.157",465);
sock2pc.connect();
sock2pc.send ("20240404");

std::vector <uint8_t> sentt={2,0,2,4,0,4,0,4};
sock2pc.send (sentt);
```

## ModbusClient类
ModbusClient是Networking命名空间下的类，用于实现MODBUS协议的客户端功能，主要用于与工业自动化设备进行通信。ModbusClient类接受IP地址、端口号以及设备的slave ID作为参数，在构造函数中初始化这些值。

readRegisters和writeRegister成员函数用于读取和写入MODBUS服务器上的寄存器数据，这是MODBUS协议中最常见的操作之一，用于获取或设置设备的状态和配置。在构造函数中不仅需要提供IP地址和端口号，还需要提供slave ID，这使得该客户端能够与多个不同的MODBUS设备进行通信，只需在创建实例时指定不同的slave ID即可。

readRegisters和writeRegister函数通过抛出std::runtime_error异常来处理通信错误，这使得错误处理更为集中且易于捕获和处理。

### 调用方法：
```c++
Networking::ModbusClient mod2plc("172.16.26.170", 504,2);
mod2plc.connect();
try {
    mod2plc.writeRegister (0,1);
    mod2plc.writeRegister (1,250);
    mod2plc.writeRegister (2,251);
    mod2plc.writeRegister (3,257);
} catch (const std::runtime_error& e) {
    std::cerr << "Failed to write register: " << e.what() << std::endl;
}
```

## Calib手眼标定类
Calib是Transform命名空间下的类，通过观察第一组的标定结果，不难发现直接对准书签，最后的标定结果中，宽度方向的结果相对准确，但长边的结果并不令人满意，初步推断时因为长边对准的误差比较大，因此我们组改用标定板进行标定，相机通过识别标定板来获取最准确的相机坐标，然后将机械臂的洗盘对准标定板的正中心，这可以保证最大程度上的标定准确。标定完成后程序会自动生成`calibration.yaml` 其内容如下：
```yaml
Calibration Data:
  - id: 1
    Camera: [1904.340820, 2396.658203]
    Arm: [840.358000, 129.369000]
  - id: 2
    Camera: [1431.669800, 2367.304932]    
    Arm: [748.168030, 131.673996]
  - id: 3
    Camera: [1257.234253, 1833.845093]    
    Arm: [714.164001, 232.763000]
```
接着通过python进行最小二乘拟合，可以拟合出最符合机械臂-相机之间的转换矩阵，之后即可直接将线性关系直接输入函数
### 调用方法：

```c++
Transform::Calib calib;
calib.img.copyto(srcImage);//回调函数
calib.calibfunc();//main函数
```