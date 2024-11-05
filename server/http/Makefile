# Specify the compiler to use
CC = cc

# Specify the directories
SRC_DIR = src
INCLUDE_DIR = include/
BIN_DIR = bin
PROGRAM_NAME = server

# Recursively find all source and header files
SRC_FILES = $(shell find $(SRC_DIR) -type f -name '*.c')
HEADER_FILES = $(shell find $(INCLUDE_DIR) -type f -name '*.h')

# Specify the output binary name
BINARY = $(BIN_DIR)/$(PROGRAM_NAME)

# Specify the compiler flags
CFLAGS = -Wall -I$(INCLUDE_DIR)

# Default target
all: $(BINARY)

# Compile the program
$(BINARY): $(SRC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Clean up
clean:
	rm -rf $(BIN_DIR)

# Run clang-format on all source and header files
format:
	clang-format -i $(SRC_FILES) $(HEADER_FILES)

# Phony targets
.PHONY: all clean format
