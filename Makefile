# Compiler and Flags
CC       := gcc
CFLAGS   := -Wall -Wextra -g -Iinclude
LDFLAGS  := 

# Directories
SRC_DIR  := src
OBJ_DIR  := object
BIN_DIR  := .

# Target Executable Name
TARGET   := $(BIN_DIR)/my_program

# Find all .c files in src/ and map them to .o files in object/
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default Rule (Builds the executable)
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets to prevent conflicts with files of the same name
.PHONY: all clean