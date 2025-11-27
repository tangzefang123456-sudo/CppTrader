#!/bin/bash

# 创建构建目录
mkdir -p build
cd build

# 生成Makefile
echo "Generating Makefile..."
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..

# 编译项目
echo "Building project..."
make -j$(nproc)

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "Build completed successfully!"
    
    # 运行单元测试
    echo "Running unit tests..."
    ./MemoryPoolTest
else
    echo "Build failed!"
    exit 1
fi

cd ..
