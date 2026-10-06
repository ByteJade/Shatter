CC=gcc
XX=g++
CFLAGS=-Wall -Wextra -O3
LIBFLAGS=-fPIC -shared -O3

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib

all: directories $(BUILD_DIR)/shatter $(LIB_DIR)/my_libc.so.6 $(LIB_DIR)/my_libm.so.6 $(LIB_DIR)/my_libX11.so.6 $(LIB_DIR)/my_libGL.so.1 $(LIB_DIR)/my_libpthread.so.0 $(LIB_DIR)/my_libstdc++.so.6 $(LIB_DIR)/my_libpng16.so.16 $(LIB_DIR)/my_libjpeg.so.8

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

$(LIB_DIR)/my_libpthread.so.0: lib/my_libpthread.c
	$(CC) $(LIBFLAGS) $< -o $@ -lpthread

$(LIB_DIR)/my_libstdc++.so.6: lib/my_libstdc++.cpp
	$(XX) $(LIBFLAGS) $< -o $@

$(LIB_DIR)/my_libpng16.so.16: lib/my_libpng16.c
	$(CC) $(LIBFLAGS) $< -o $@ -lpng

$(LIB_DIR)/my_libjpeg.so.8: lib/my_libjpeg.c
	$(CC) $(LIBFLAGS) $< -o $@ -ljpeg

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all directories clean
