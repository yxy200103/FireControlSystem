cmake_minimum_required(VERSION 3.16)
project(detector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_USE_RELATIVE_PATHS ON)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#set(OpenCV_DIR ${PROJECT_SOURCE_DIR}/OpenCV/lib/cmake/opencv4)
#set(OpenCV_LIBS ${PROJECT_SOURCE_DIR}/OpenCV/lib)
#set(OpenCV_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/OpenCV/include)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message("OpenCV FOUND")
    # message(${OpenCV_LIBS})
endif()

include_directories(NCNN)

set(NCNN_DIR ${PROJECT_SOURCE_DIR}/NCNN/lib/cmake/ncnn)
set(NCNN_LIBS ${PROJECT_SOURCE_DIR}/NCNN/lib/)
set(NCNN_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/NCNN/include)
set(NCNN_LIB_FILES ncnn glslang OGLCompiler OSDependent SPIRV)

find_package(NCNN REQUIRED)
if(NCNN_FOUND)
    message("NCNN FOUND")
endif()

include_directories(
        # "-Wl,--whole-archive"
        ${OpenCV_INCLUDE_DIRS}
        ${NCNN_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(detector SHARED src/library.cpp src/library.h src/detector.cpp src/nanodet.cpp)

link_directories(
        ${NCNN_LIBS}
)

set(BUILD_SHARED_LIB ON)

target_link_libraries(
        detector
        ${NCNN_LIB_FILES}
        ${OpenCV_LIBS}
)
