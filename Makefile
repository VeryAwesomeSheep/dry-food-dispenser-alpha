# Compiler
CC := gcc

# Directories
SRC_DIR := .
LIBS_DIR := libs

# Source files
SRC := $(SRC_DIR)/feeder.c
LIBS_SRC := $(wildcard $(LIBS_DIR)/*.c)

# Object files
OBJ := $(SRC:.c=.o)
LIBS_OBJ := $(LIBS_SRC:.c=.o)

# Compiler flags
CFLAGS := -Wall

# Libraries
LIB := -lwiringPi

# Target
TARGET := feeder.out

# Default target
all: $(TARGET) clean

# Default target without cleaning
allnc: $(TARGET)

# Compile main program
$(TARGET): $(OBJ) $(LIBS_OBJ)
	$(CC) $(CFLAGS) $^ $(LIB) -o $@

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJ) $(LIBS_OBJ)

# Clean target
cleanall:
	rm -f $(OBJ) $(LIBS_OBJ) $(TARGET)