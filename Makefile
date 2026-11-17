CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -m64 -Iinclude -Xpreprocessor -fopenmp -Wno-unknown-pragmas

.PHONY: all clean

all: bin/converter

bin/converter: apps/converter.cpp build/csr.o
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

build/csr.o: src/csr.cpp include/csr.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build bin
