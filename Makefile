# Compiler and Flags
CXX      := gcc
CXXFLAGS := -std=c17 -Wall -Wextra
LDFLAGS  := 

# Add this to CXXFLAGS
CXXFLAGS += -MMD -MP

# Add this to the bottom of the Makefile
-include $(OBJECTS:.o=.d)

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Files
# This automatically finds all .c files in src/ and main.c
SOURCES  := $(wildcard $(SRC_DIR)/*.c) main.c
# This converts src/FileName.c to obj/FileName.o
OBJECTS  := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET   := $(BIN_DIR)/brainf

# Phony Targets (commands that aren't files)
.PHONY: all clean run

all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
