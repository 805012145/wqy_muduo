#!/bin/bash
cd build
# 删除 build 目录下所有文件
rm -rf *
# 运行 cmake 和 make
cmake ..
make -j$(nproc)