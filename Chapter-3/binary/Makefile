CUR_DIR = $(shell pwd)
LLVM_SRC := ${CUR_DIR}/../../../llvm
#cmd:
#clang++ -g toy.cpp `../../../llvm/build/bin/llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native` -O3 -o toy

CC = clang++
SOURCE = toy.cpp
TARGET = toy

$(TARGET) : $(SOURCE)
	$(CC) -g $(SOURCE) `../../../llvm/build/bin/llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native` -O3 -o $(TARGET)

clean :
	rm $(TARGET)
