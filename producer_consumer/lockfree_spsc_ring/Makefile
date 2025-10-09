# --- Makefile for LockFreeSPSCRingBuffer ---

# Define the C++ compiler to use
CXX = g++

# Define the name of the executable file
TARGET = producer_consumer

# Define the source file(s)
SOURCES = producer_consumer.cpp

# --- Compiler Flags ---

# -std=c++17: Ensures support for modern C++ features like std::atomic
# -Wall -Wextra: Enables all common and extra warnings (Good practice!)
# -pedantic: Enforces strict standards conformance
# -O3: Aggressive optimization (Crucial for high-performance finance code)
# -DNDEBUG: Disables assert() statements, useful for production builds
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3 -DNDEBUG

# -pthread: Required for linking with multithreading support (std::thread)
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
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Phony targets: Targets that do not represent a file
.PHONY: all clean run
