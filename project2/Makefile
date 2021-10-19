CC = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
INC = include/
CPPFLAGS += -I$(INC)
TARGET = run

all: $(TARGET)

$(TARGET):
	$(foreach file, $(SRCS), $(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(file:.cpp=.o) -c $(file);)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm $(TARGET) $(OBJS)