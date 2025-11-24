cmake_minimum_required(VERSION 3.16)
project(ExceptionCatcher VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
endif()

# 查找GTest库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(common)

# 创建异常捕获组件库
add_library(ExceptionCatcher STATIC
    common/exception_catcher.h
    common/exception_catcher.cpp
)

# 创建单元测试可执行文件
add_executable(TestExceptionCatcher
    tests/test_exception_catcher.cpp
)

# 链接依赖库
target_link_libraries(TestExceptionCatcher
    ExceptionCatcher
    GTest::GTest
    GTest::Main
)

# 启用测试
enable_testing()
add_test(NAME TestExceptionCatcher COMMAND TestExceptionCatcher)
