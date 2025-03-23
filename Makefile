# Define the build directory
BUILD_DIR = build

.PHONY: all test build debug clean

# Default target
all: build test

# Target to configure the project
configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

# Target to build the project
build:
	cmake --build $(BUILD_DIR)

# Target to rebuild the project
rebuild: clean configure build

# Target to run the xdb executable
run:
	./$(BUILD_DIR)/tools/xdb

# Target to clean the build directory
clean:
	rm -rf $(BUILD_DIR)

# Target to run the test script
test:
	./test/test.sh