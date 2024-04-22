cmake_minimum_required(VERSION 3.10)
project(Group2)

set(CMAKE_CXX_STANDARD 14)
set(OPENCV_PATH "${CMAKE_SOURCE_DIR}/libs/OpenCV")


add_executable(Group2 main.cpp main.h hik_cam/hik_camera.cpp hik_cam/hik_camera.h)

include_directories("${OPENCV_PATH}/include")
target_link_libraries(Group2 "${OPENCV_PATH}/opencv_world455.lib")
target_link_libraries(Group2 "${CMAKE_SOURCE_DIR}/libs/MvCameraControl.lib")




