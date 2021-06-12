#!/bin/bash

sudo apt update
sudo apt install gcc g++ make cmake

# install opencv
sudo apt install libopencv

# install vulkan
wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list http://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list
sudo apt update
sudo apt install vulkan-sdk

# install ncnn
git clone https://github.com/Tencent/ncnn.git
cd ncnn
git clone https://github.com/KhronosGroup/glslang.git

mkdir build
cd build

# build ncnn
cmake -DCMAKE_BUILD_TYPE=Release -DNCNN_VULKAN=ON -DNCNN_SYSTEM_GLSLANG=ON -DNCNN_BUILD_EXAMPLES=ON ..
make -j($nproc)
make install

# copy ncnn libraries and headers
cp -r install ../Detector/
mv install ncnn

# compile and build
mkdir build
cd build
cmake ..
make

cd ../.. # root dir of project
cd Communicator

# install dotnet requirement
dotnet add package SerialPort
dotnet build

cd .. # root dir
cp ./Detector/build/libdetector.so ./Communicator
