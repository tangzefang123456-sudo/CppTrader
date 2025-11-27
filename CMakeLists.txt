cmake_minimum_required(VERSION 3.16)
project(MemoryPool VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX /MP)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 包含目录
include_directories(include)

# 内存池库
add_library(MemoryPool STATIC
    src/common/memory_pool.cpp
    include/common/memory_pool.h
)

# 目标属性
set_target_properties(MemoryPool PROPERTIES
    PUBLIC_HEADER include/common/memory_pool.h
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 检查是否要构建测试
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    FetchContent_MakeAvailable(googletest)

    # 单元测试可执行文件
    add_executable(MemoryPoolTest
        tests/memory_pool_test.cpp
    )

    # 链接库
    target_link_libraries(MemoryPoolTest
        PRIVATE MemoryPool
        PRIVATE gtest_main
    )

    # 测试
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(MemoryPoolTest)
endif()

# 安装规则
install(TARGETS MemoryPool
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/common
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(BUILD_TESTS)
    install(TARGETS MemoryPoolTest
        RUNTIME DESTINATION bin
    )
endif()
