FLAGS=-O3 -march=native #-g -fsanitize=address
CC=g++
BIN_DIR = bin
BENCHTOOL ?= USE_RDTSC

# List of binaries
BINARIES = benchmark-1 benchmark-2 benchmark-3 benchmark-4

# Ensure bin/ exists before compiling
all: $(BIN_DIR) $(patsubst %, $(BIN_DIR)/%, $(BINARIES))

# Rule to create bin/ directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
	mkdir -p data

# Compile each benchmark with different TWEAK_BLOCKS values
$(BIN_DIR)/benchmark-%: benchmark.cpp | $(BIN_DIR)
	$(CC) $(FLAGS) -DTWEAK_BLOCKS=$* -D$(BENCHTOOL) benchmark.cpp -o $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)
clean-all:
	rm -rf $(BIN_DIR)
	rm -rf data

