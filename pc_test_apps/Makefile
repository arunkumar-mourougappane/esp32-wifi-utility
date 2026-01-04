CXX = g++
CXXFLAGS = -O3 -Wall -pthread
TARGET = udp_echo_server
SRC = udp_echo_server.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
