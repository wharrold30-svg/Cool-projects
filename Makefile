# ==============================================================================
# Pitchfork Layout Makefile Standard
# ==============================================================================

# Compiler and Flags
CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2

# 1. Pitchfork Directory Definitions
SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/obj
BIN_DIR     := $(BUILD_DIR)/apps

# Target Binary Name
TARGET      := $(BIN_DIR)/my_project_app

# Include Paths (Informs compiler about public include/ and private src/ headers)
INCLUDES    := -I$(INCLUDE_DIR) -I$(SRC_DIR)

# 2. Source and Object File Discovery
SRCS        := $(wildcard $(SRC_DIR)/*.cpp)
OBJS        := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

# ==============================================================================
# Build Rules
# ==============================================================================

.PHONY: all clean directories

# Default target
all: directories $(TARGET)

# Create Pitchfork build directories safely
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Link the final application binary
$(TARGET): $(OBJS)
	@echo "Linking executable: $@"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# Compile source files to object files (Generates auto-dependency tracking .d files)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include auto-generated dependency files (handles header changes)
-include $(DEPS)

# Clean up all build artifacts
clean:
	@echo "Cleaning Pitchfork build artifacts..."
	rm -rf $(BUILD_DIR)

