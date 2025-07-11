CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -O2
TARGET = hello
SOURCE = hello.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: clean