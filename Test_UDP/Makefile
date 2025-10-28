# =========================
# 项目信息
# =========================
CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2
LDFLAGS := -lpthread

# MiniCore 库路径
MINICORE_DIR := ../MiniCore
LIBS := $(MINICORE_DIR)/build/libIceLog.a
INCLUDES := -I. -I../MiniCore/include -I../MiniCore/include/icelog


# =========================
# 源文件与目标
# =========================
TARGET_SERVER := udp_server
TARGET_CLIENT := udp_client

SRCS_SERVER := UDP_server.cpp
SRCS_CLIENT := UDP_client.cpp
OBJS_SERVER := $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT := $(SRCS_CLIENT:.cpp=.o)

# =========================
# 默认目标
# =========================
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# =========================
# 编译规则
# =========================
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# =========================
# 清理
# =========================
clean:
	rm -f *.o $(TARGET_SERVER) $(TARGET_CLIENT)

.PHONY: all clean
