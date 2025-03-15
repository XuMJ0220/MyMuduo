cmake_minimum_required(VERSION 2.5)
project(mymuduo)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# mymuduo最终编译成so动态库 设置动态库的路径，放在根目录的lib目录下
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)
#编译编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})

