cmake_minimum_required(VERSION 3.10)
project(Group2)

set(CMAKE_CXX_STANDARD 14)
set(Lib_PATH "${CMAKE_SOURCE_DIR}/Libs")


add_executable(Group2 main.cpp main.h HikCam/hik_camera.cpp HikCam/hik_camera.h
                Detector/Detector.cpp Detector/Detector.h
        Detector/box.h)

include_directories("${CMAKE_SOURCE_DIR}/Includes")

target_link_libraries(Group2 "${Lib_PATH}/MvCameraControl.lib")
target_link_libraries(Group2 "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/WS2_32.Lib")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(Group2 "${Lib_PATH}/opencv_world455d.lib")
    target_link_libraries(Group2 "${Lib_PATH}/yaml-cppd.lib")
    target_link_libraries(Group2 "${Lib_PATH}/modbusd.lib")
else()
    target_link_libraries(Group2 "${Lib_PATH}/opencv_world455.lib")
    target_link_libraries(Group2 "${Lib_PATH}/yaml-cpp.lib")
    target_link_libraries(Group2 "${Lib_PATH}/modbus.lib")
endif()

