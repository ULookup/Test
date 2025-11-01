# 编译器与编译选项
CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2

# 目录与文件
SRC_DIRS := . InetAddressHttp SocketHttp
SRCS := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
OBJS := $(SRCS:.cpp=.o)
TARGET := Server

# 规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
