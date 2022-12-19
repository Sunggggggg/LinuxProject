# 컴파일러
CC = gcc
CPP = g++

# C++ 컴파일러 옵션
CXXFLAGS = -Wall -O2

# 생성하고자 하는 실행 파일 이름 (빌드 대상 이름)
TARGET = head_pose

OBJECTS = hog.o ldmarkmodel.o head_pose.o
DEPS = $(OBJECTS:.o=.d)

all : head_pose

# 컴파일 & 링크
hog.o : ./src/hog.c
	$(CC) $(CXXFLAGS) -c  $< -o $@ -MD

ldmarkmodel.o : ./src/ldmarkmodel.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@ -MD

head_pose.o : ./src/head_pose.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@ -MD

$(TARGET) : $(OBJECTS)
	$(CPP) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(pkg-config opencv4 --libs --cflags)

.PHONY : clean all

clean :
	rm -f $(OBJECTS) $(DEPS) $(TARGET)

-include $(DEPS)