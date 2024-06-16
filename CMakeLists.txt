cmake_minimum_required(VERSION 3.10)
project(Group2)

set(CMAKE_CXX_STANDARD 14)
set(Lib_PATH "${CMAKE_SOURCE_DIR}/Libs")


add_executable(Group2 main.cpp
        main.h HikCam/hik_camera.cpp HikCam/hik_camera.h
                Detector/Detector.cpp Detector/Detector.h
        Detector/box.h
        Modbus/ModbusClient.cpp
        Modbus/ModbusClient.h
        Socket/SocketClient.cpp
        Socket/SocketClient.h
        HandEye/HandEyeTransmit.cpp
        HandEye/HandEyeTransmit.h
)

include_directories("${CMAKE_SOURCE_DIR}/Includes")


target_link_libraries(Group2 "${Lib_PATH}/MvCameraControl.lib")

target_link_libraries(Group2 "${Lib_PATH}/modbus.lib")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(Group2 "${Lib_PATH}/opencv_world455d.lib")
    target_link_libraries(Group2 "${Lib_PATH}/yaml-cppd.lib")
else()
    target_link_libraries(Group2 "${Lib_PATH}/opencv_world455.lib")
    target_link_libraries(Group2 "${Lib_PATH}/yaml-cpp.lib")
endif()

