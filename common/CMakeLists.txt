cmake_minimum_required(VERSION 3.16)
project(ThreadBase)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找gtest库
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 创建ThreadBase库
add_library(ThreadBase STATIC ThreadBase.cpp ThreadBase.h)

# 创建测试可执行文件
add_executable(ThreadBaseTest ThreadBaseTest.cpp)

# 链接库
target_link_libraries(ThreadBaseTest ThreadBase ${GTEST_LIBRARIES} pthread)

# 设置输出目录
set_target_properties(ThreadBase PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

set_target_properties(ThreadBaseTest PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)
