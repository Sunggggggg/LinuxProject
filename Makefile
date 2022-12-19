# �����Ϸ�
CC = gcc
CPP = g++

# C++ �����Ϸ� �ɼ�
CXXFLAGS = -Wall -O2

# �����ϰ��� �ϴ� ���� ���� �̸� (���� ��� �̸�)
TARGET = head_pose

OBJECTS = hog.o ldmarkmodel.o head_pose.o
DEPS = $(OBJECTS:.o=.d)

all : head_pose

# ������ & ��ũ
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