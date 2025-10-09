# --- Makefile for Lock-Free SPSC Queue Demo ---

# Define the C++ compiler to use
CXX = g++

# Define the name of the executable file
TARGET = producer_consumer

# Define the source file(s)
# Assume the provided code is saved in this file:
SOURCES = producer_consumer.cpp

# --- Compiler Flags ---

# -std=c++17: Required for modern C++ features (std::atomic, std::thread, etc.)
# -Wall -Wextra: Enable extensive warnings (good practice)
# -O3: Aggressive optimization (essential for high-performance/finance code)
# -DNDEBUG: Disable assert() statements if they were used (standard for optimized builds)
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG

# -pthread: Required for linking with multithreading support (std::thread, std::mutex)
LDFLAGS = -pthread

# --- Core Targets ---

# Default target: Builds the executable
all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
	@echo "Successfully compiled $(TARGET)."

# Clean target: Removes the generated executable
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)
	@echo "Clean complete."

# Run target: Builds the executable then runs it
run: $(TARGET)
	@echo "Running $(TARGET)... (Press Ctrl+C to stop)"
	./$(TARGET)

# Phony targets: Targets that do not represent a file
.PHONY: all clean run
