# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -g -pthread


# Directoriess
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Header files
DEPS = $(wildcard $(INC_DIR)/*.h)

# Target executable
TARGET = $(BIN_DIR)/blockchain_test

# OPEN SSL flags
OPENSSL_CFLAGS = $(shell pkg-config --cflags openssl)
OPENSSL_LIBS = $(shell pkg-config --libs openssl)

# Phony targets
.PHONY: all clean run test debug rebuild

# Default target
all: $(TARGET)

# Rule to create object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(OPENSSL_CFLAGS) -I$(INC_DIR) -c $< -o $@

# Rule to create the executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ $(OPENSSL_LIBS) -o $@

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run target
run: $(TARGET)
	./$(TARGET) 8080

# Test target
test: run

# rebuild target
rebuild: clean all

# Debug information
debug:
	@echo "Source files:"
	@echo $(SRCS)
	@echo "Object files:"
	@echo $(OBJS)
	@echo "Dependencies:"
	@echo $(DEPS)

# Error checking
$(SRCS):
	$(error Source file $@ not found. Make sure your .c files are in the $(SRC_DIR) directory)

$(DEPS):
	$(error Header file $@ not found. Make sure your .h files are in the $(INC_DIR) directory)
