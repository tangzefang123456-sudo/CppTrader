//
// Created by Your Name on 2024-05-20
//

#include "test.h"
#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <fstream>
#include <filesystem>

using namespace Trader::Common;

TEST_CASE("ExceptionHandler initialization", "[Trader][Common][ExceptionHandler]")
{
    ExceptionHandler& handler = ExceptionHandler::GetInstance();

    // Test default initialization
    REQUIRE(handler.Init("exception_test.log"));
    REQUIRE(handler.Start());
    REQUIRE(handler.Stop());

    // Test re-initialization
    REQUIRE(!handler.Init("exception_test.log"));

    // Test start without initialization
    ExceptionHandler& handler2 = ExceptionHandler::GetInstance();
    REQUIRE(handler2.Stop()); // Ensure it's stopped first
    REQUIRE(!handler2.Start());
}

TEST_CASE("ExceptionHandler cleanup callbacks", "[Trader][Common][ExceptionHandler]")
{
    ExceptionHandler& handler = ExceptionHandler::GetInstance();
    REQUIRE(handler.Init("exception_test.log"));
    REQUIRE(handler.Start());

    std::atomic<int> callback1_called = 0;
    std::atomic<int> callback2_called = 0;

    // Register cleanup callbacks
    auto callback1 = [&]() {
        callback1_called++;
    };

    auto callback2 = [&]() {
        callback2_called++;
    };

    handler.RegisterCleanupCallback(callback1);
    handler.RegisterCleanupCallback(callback2);

    // Execute callbacks manually for testing
    handler.ExecuteCleanupCallbacks();

    REQUIRE(callback1_called == 1);
    REQUIRE(callback2_called == 1);

    // Unregister one callback and execute again
    handler.UnregisterCleanupCallback(callback1);
    handler.ExecuteCleanupCallbacks();

    REQUIRE(callback1_called == 1);
    REQUIRE(callback2_called == 2);

    REQUIRE(handler.Stop());
}

TEST_CASE("ExceptionHandler stack trace", "[Trader][Common][ExceptionHandler]")
{
    ExceptionHandler& handler = ExceptionHandler::GetInstance();
    REQUIRE(handler.Init("exception_test.log"));

    // Get stack trace
    std::string stack_trace = handler.GetStackTrace();
    REQUIRE(!stack_trace.empty());
    REQUIRE(stack_trace.find("GetStackTrace") != std::string::npos);

    REQUIRE(handler.Stop());
}

// Note: We can't easily test actual signal handling in unit tests because it would crash the test process
// For integration testing, you can create a separate executable that intentionally causes a segfault
