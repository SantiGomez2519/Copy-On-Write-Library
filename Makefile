CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRCS = main.cpp src/file_manager.cpp src/storage.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = versioning_app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)                             

clean:
	rm -f $(OBJS) $(TARGET) *.meta *.blocks *.data

.PHONY: all run clean