cmake_minimum_required(VERSION 3.16)
project(mymuduo)

# so 动态库放在根目录lib文件夹下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMKAE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

# c++11 语言标准
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBS pthread)

add_subdirectory(src)