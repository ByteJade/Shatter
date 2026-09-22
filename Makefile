CXX=g++
CXXFLAGS=-Wall -Wextra -O3
LIBFLAGS=-fPIC -shared -O3

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib

all: directories $(BUILD_DIR)/shatter $(LIB_DIR)/my_libc.so.6 $(LIB_DIR)/my_libm.so.6 $(LIB_DIR)/my_libX11.so.6 $(LIB_DIR)/my_libGL.so.1

directories:
	mkdir -p $(LIB_DIR)

$(BUILD_DIR)/shatter: src/*
	$(CXX) $(CXXFLAGS) src/* -o $@ -lreadline

$(LIB_DIR)/my_libc.so.6: libraries/my_libc.cpp
	$(CXX) $(LIBFLAGS) $< -o $@

$(LIB_DIR)/my_libm.so.6: libraries/my_libm.cpp
	$(CXX) $(LIBFLAGS) $< -o $@ -lm

$(LIB_DIR)/my_libX11.so.6: libraries/my_libX11.cpp
	$(CXX) $(LIBFLAGS) $< -o $@ -lX11

$(LIB_DIR)/my_libGL.so.1: libraries/my_libGL.cpp
	$(CXX) $(LIBFLAGS) $< -o $@ -lGL

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all directories clean