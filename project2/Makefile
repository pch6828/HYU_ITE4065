CC = g++
CXXFLAGS = -g -Wall -std=c++17
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
INC = include/
CPPFLAGS += -I$(INC)
TARGET = run

all: $(TARGET)

$(TARGET):
	$(foreach file, $(SRCS), $(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(file:.cpp=.o) -c $(file) -lpthread;)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS) -lpthread

clean:
	rm $(TARGET) $(OBJS)