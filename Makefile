CC = gcc
CPP = g++

CXXFLAGS = -Wall -O2

# 소스 파일 디렉토리
SRC_DIR = ./src

# 오브젝트 파일 디렉토리
OBJ_DIR = ./obj

# 생성하고자 하는 실행 파일 이름 (빌드 대상 이름)
TARGET = head_pose

# MAKE 할 소스 파일들
# wildcard 로 SRC_DIR에서 *.cc로 된 파일들 목록을 뽑아낸 뒤에
# notdir로 파일 이름만 뽑아낸다.
# (e.g. SRCS는 foo.cc bar.cc main.cc가 된다.)
SRCS = $(notdir $(wildcard $(SRC_DIR)/*.c))

OBJS = $(SRCS:.c=.o)

# OBJS 안의 object 파일들 이름 앞에 $(OBJ_DIR)/ 을 붙인다.
OBJECTS = $(patsubst %.o, $(OBJ_DIR)/%.o, $(OBJS))
DEPS = $(OBJECTS:.o=.d)

all : head_pose

# 컴파일 & 링크
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CXXFLAGS) -c  $< -o $@ -MD

./obj/ldmarkmodel.o : ./src/ldmarkmodel.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@ -MD

./obj/head_pose.o : ./src/head_pose.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@ -MD

$(TARGET) : $(OBJECTS)
	$(CPP) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(pkg-config opencv4 --libs --cflags)

.PHONY : clean all

clean :
	rm -f $(OBJECTS) $(DEPS) $(TARGET)

-include $(DEPS)