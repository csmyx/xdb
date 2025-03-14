# Define the build directory
BUILD_DIR = build

.PHONY: all test build debug clean

# Default target
all: build test

# Target to configure the project
configure:
	cmake -S . -B $(BUILD_DIR)

# Target to build the project
build:
	cmake --build $(BUILD_DIR)

# Target to clean the build directory
clean:
	rm -rf $(BUILD_DIR)

# Target to run the test script
test:
	./test/test.sh