# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Directories
SRCDIR = src
BUILDDIR = build

# Source files
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/util.c $(SRCDIR)/lexer.c $(SRCDIR)/list.c $(SRCDIR)/map.c $(SRCDIR)/assembler.c

# Object files
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Target executable
TARGET = $(BUILDDIR)/g1a

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Link object files to create executable
$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CC) $(OBJECTS) -o $@

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)

# Rebuild everything
rebuild: clean all

# Mark targets that don't create files
.PHONY: all clean rebuild