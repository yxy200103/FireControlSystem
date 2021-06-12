cmake_minimum_required(VERSION 3.19)
project(Energy)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS}/include include)

add_executable(Energy src/energy.cpp include/threshold.h src/threshold.cpp src/detect.cpp include/detect.h include/config.h src/colourdetect.cpp include/colourdetect.h src/library.cpp)

target_link_libraries(
        Energy
        ${OpenCV_LIBS}
)
