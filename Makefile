CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

TARGET = karatsuba_app
SRCS = src/operaciones.cpp src/karatsuba.cpp src/main_pruebas.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)