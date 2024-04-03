# Define the compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Linker flags
LDFLAGS = -lreadline

# Source files
SOURCES = kinshell.c shellloop.c builtin.c parse.c execute.c completion.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Output binary
TARGET = kinshell

# Default target
all: $(TARGET)

# Rule for linking the final binary
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Rule for compiling source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for cleaning
clean:
	rm -f $(TARGET) $(OBJECTS)