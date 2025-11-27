# 编译器
CXX = g++
# C++标准
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror
# 包含目录
INCLUDES = -Iinclude
# 目标文件
TARGET = simple_test
# 源文件
SOURCES = simple_test.cpp src/common/memory_pool.cpp
# 头文件
HEADERS = include/common/memory_pool.h

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES)

# 清理
clean:
	rm -f $(TARGET)

# 运行测试
run:
	./$(TARGET)

.PHONY: all clean run
