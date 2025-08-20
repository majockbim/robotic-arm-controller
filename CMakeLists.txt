cmake_minimum_required(VERSION 3.10)

project(robotic_arm_controller)

set(OpenCV_DIR C:/opencv/build)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} src/gestureController.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})