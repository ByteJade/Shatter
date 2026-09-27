CC=gcc
CFLAGS=-Wall -Wextra -O3
LIBFLAGS=-fPIC -shared -O3

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib

all: directories $(BUILD_DIR)/shatter $(LIB_DIR)/my_libc.so.6 $(LIB_DIR)/my_libm.so.6 $(LIB_DIR)/my_libX11.so.6 $(LIB_DIR)/my_libGL.so.1

directories:
	mkdir -p $(LIB_DIR)

$(BUILD_DIR)/shatter: src/*
	$(CC) $(CFLAGS) src/* -o $@

$(LIB_DIR)/my_libc.so.6: lib/my_libc.c
	$(CC) $(LIBFLAGS) $< -o $@

$(LIB_DIR)/my_libm.so.6: lib/my_libm.c
	$(CC) $(LIBFLAGS) $< -o $@ -lm

$(LIB_DIR)/my_libX11.so.6: lib/my_libX11.c
	$(CC) $(LIBFLAGS) $< -o $@ -lX11

$(LIB_DIR)/my_libGL.so.1: lib/my_libGL.c
	$(CC) $(LIBFLAGS) $< -o $@ -lGL

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all directories clean
