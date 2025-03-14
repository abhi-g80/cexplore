# Specify the compiler to use
CC = cc

# Specify the directories
SRC_DIR = src
BIN_DIR = bin
PROGRAM_NAME = webby

# Recursively find all source and header files
SRC_FILES = $(shell find $(SRC_DIR) -type f -name '*.c')
HEADER_FILES = $(shell find $(INCLUDE_DIR) -type f -name '*.h')

# Specify the output binary name
BINARY = $(BIN_DIR)/$(PROGRAM_NAME)

GENFLAGS = -Wall -std=gnu2x

# Specify the compiler flags
CFLAGS = $(GENFLAGS) -O2

# Specify debug build compiler flags
DEBUGCFLAGS = $(GENFLAGS) -g

# Default target
all: $(BINARY)

# Compile the program
$(BINARY): $(SRC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^


# Compile the program
debug: $(SRC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(DEBUGCFLAGS) -o $(BINARY) $^

# Clean up
clean:
	rm -rf $(BIN_DIR)

# Run clang-format on all source and header files
format:
	clang-format -i $(SRC_FILES) $(HEADER_FILES)

# Run docker build
docker-build:
	docker build -t $(PROGRAM_NAME)-test .

# Leak check with Valgrind 
leak-check: debug
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all bin/webby -d	

# Phony targets
.PHONY: all clean format docker-build leak-check
