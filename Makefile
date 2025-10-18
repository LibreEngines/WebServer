# Makefile for Advanced C Web Server

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lpthread

# Source files
SOURCES = main.c http_server.c router.c template.c logger.c utils.c auth.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = webserver

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) server.log


# Publish to git
publish:
	make clean && git add . && git commit -m "CORE" && git push

# Install target (optional)
install: $(TARGET)
	mkdir -p /usr/local/bin
	cp $(TARGET) /usr/local/bin/

# Create necessary directories
setup:
	mkdir -p templates static logs

# Run the server
run: $(TARGET) setup
	./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Check for memory leaks (requires valgrind)
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Static analysis (requires cppcheck)
analyze:
	cppcheck --enable=all --std=c99 $(SOURCES)

# Format code (requires clang-format)
format:
	clang-format -i *.c *.h

# Dependencies
main.o: main.c http_server.h router.h logger.h template.h auth.h
http_server.o: http_server.c http_server.h logger.h utils.h router.h
router.o: router.c router.h http_server.h template.h logger.h utils.h
template.o: template.c template.h logger.h utils.h
logger.o: logger.c logger.h
utils.o: utils.c utils.h logger.h
auth.o: auth.c auth.h logger.h utils.h http_server.h

.PHONY: all clean install setup run debug release memcheck analyze format

