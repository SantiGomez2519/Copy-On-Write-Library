CXX = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude

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
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean