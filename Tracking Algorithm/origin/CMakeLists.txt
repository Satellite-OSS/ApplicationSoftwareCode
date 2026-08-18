cmake_minimum_required(VERSION 3.10)
project(OVX1_Implementation_CN) 

set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(OpenCV REQUIRED)


message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS}) 

add_executable(ovx1_app_cn main.cpp) 

target_link_libraries(ovx1_app_cn ${OpenCV_LIBS}) 