BIN=test
CC=g++
CPPFLAG=-g
STD=-std=c++11
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o) #将SRC所有同名.cpp替换成为.o形成目标文件列表
LFLAGS=-o #Link 连接选项
CFLAGS=-c #Compile 编译选项
RM=rm -f 
$(BIN):$(OBJ)
	@$(CC) $(STD) $(CPPFLAG) $(LFLAGS) $@ $^ 
	@echo "链接 $^ 成为 $@"
%.o:%.cpp
	@$(CC) $(STD) $(CPPFLAG) $(CFLAGS) $<
	@echo "编译$< 成为 $@"

.PHONY:clean
clean:
	@$(RM) $(BIN) $(OBJ)
	@echo "清理完毕"
