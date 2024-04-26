cmake_minimum_required(VERSION 3.10)
project(Group2)

set(CMAKE_CXX_STANDARD 14)
set(OPENCV_PATH "${CMAKE_SOURCE_DIR}/Libs/OpenCV")


add_executable(Group2 main.cpp main.h HikCam/hik_camera.cpp HikCam/hik_camera.h
                Detector/Detector.cpp Detector/Detector.h
        Detector/box.h)

include_directories("${OPENCV_PATH}/include")
target_link_libraries(Group2 "${OPENCV_PATH}/opencv_world455d.lib")
target_link_libraries(Group2 "${CMAKE_SOURCE_DIR}/Libs/MvCameraControl.lib")

