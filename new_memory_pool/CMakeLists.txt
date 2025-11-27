cmake_minimum_required(VERSION 3.20)

# Global properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Project name
project(MemoryPool)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# MemoryPool library
add_library(MemoryPool STATIC
    memory_pool.h
    memory_pool.cpp
)
set_target_properties(MemoryPool PROPERTIES FOLDER "libraries")

# MemoryPool tests
add_executable(MemoryPoolTests
    test_memory_pool.cpp
)
target_link_libraries(MemoryPoolTests MemoryPool)
set_target_properties(MemoryPoolTests PROPERTIES FOLDER "tests")

# CTest
enable_testing()
add_test(MemoryPoolTests MemoryPoolTests)

# Install
install(TARGETS MemoryPool MemoryPoolTests
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION bin
)
