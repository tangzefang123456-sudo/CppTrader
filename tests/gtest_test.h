#pragma once

// Google Test headers
#include <gtest/gtest.h>

// Common headers
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

// Initialize Google Test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
