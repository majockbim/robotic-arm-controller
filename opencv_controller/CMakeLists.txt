cmake_minimum_required(VERSION 3.10)
project(opencv_controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# point to openCV
set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

# incclude directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)


# add all source files
add_executable(opencv_controller
    src/main.cpp
    gestureController.cpp
    serial.cpp
)

# openCV libraries
target_link_libraries(opencv_controller ${OpenCV_LIBS})

# display debug info
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")