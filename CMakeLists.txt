cmake_minimum_required(VERSION 3.10)
project(Group2)

set(CMAKE_CXX_STANDARD 14)
set(Lib_PATH "${CMAKE_SOURCE_DIR}/Libs")


add_executable(Group2 main.cpp main.h HikCam/hik_camera.cpp HikCam/hik_camera.h
                Detector/Detector.cpp Detector/Detector.h
        Detector/box.h)

include_directories("${CMAKE_SOURCE_DIR}/Includes")
target_link_libraries(Group2 "${Lib_PATH}/opencv_world455.lib")
target_link_libraries(Group2 "${Lib_PATH}/MvCameraControl.lib")

