OS:=$(shell uname)
MATRIX_DIR = data/input/matrices
CACHE_DIR = data/input/cache
CHUNK_SIZE = 40m

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17 -m64 -Iinclude -Xpreprocessor -fopenmp -Wno-unknown-pragmas
ifeq ($(OS), Darwin)
CXXFLAGS += -I/opt/homebrew/opt/libomp/include
CXXFLAGS += -L/opt/homebrew/opt/libomp/lib 
CXXFLAGS += -lomp
CXXFLAGS += -Wno-unused-command-line-argument
endif

.PHONY: all clean split-csr join-csr

all: bin/converter bin/solver

bin/converter: apps/converter.cpp build/csr.o
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/solver: apps/solver.cpp build/csr.o build/sequential.o build/level.o
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

build/csr.o: src/csr.cpp include/csr.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/sequential.o: src/sequential.cpp include/sequential.h include/csr.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/level.o: src/level.cpp include/level.h include/csr.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

split-csr:
	@mkdir -p $(MATRIX_DIR)
	@for file in $(CACHE_DIR)/*.csrbin; do \
		[ -f "$$file" ] || continue; \
		name=$$(basename "$$file" .csrbin); \
		echo "Splitting $$name..."; \
		split -d -b $(CHUNK_SIZE) -a 2 "$$file" \
			"$(MATRIX_DIR)/$$name.csrbin.part"; \
	done

join-csr:
	@mkdir -p $(CACHE_DIR)
	@for file in $(MATRIX_DIR)/*.csrbin.part00; do \
		[ -f "$$file" ] || continue; \
		prefix=$${file%00}; \
		name=$$(basename "$$prefix" .part); \
		echo "Joining $$name..."; \
		cat "$${prefix}"* > "$(CACHE_DIR)/$$name"; \
	done

clean:
	rm -rf build bin
