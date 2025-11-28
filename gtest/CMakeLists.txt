# CMakeLists.txt for Google Test-based unit tests
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(OrderBookTests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用FetchContent自动下载和构建Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include_directories(${googletest_SOURCE_DIR}/googletest/include)

# 查找Threads库
find_package(Threads REQUIRED)

# 包含项目头文件
include_directories(${PROJECT_SOURCE_DIR}/../include)

# 链接项目库
link_directories(${PROJECT_SOURCE_DIR}/../bin)

# 测试源文件
set(TEST_SOURCES
    test_order_book.cpp
)

# 创建测试可执行文件
add_executable(OrderBookTests ${TEST_SOURCES})

# 链接依赖库
target_link_libraries(OrderBookTests
    GTest::gtest_main
    trader
    Threads::Threads
)

# 启用测试
enable_testing()
add_test(NAME OrderBookTests COMMAND OrderBookTests)

# 设置输出目录
set_target_properties(OrderBookTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin
)
