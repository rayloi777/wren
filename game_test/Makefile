config ?= release_64bit

CC = gcc
CFLAGS = -std=c99 -m64 -O2
INCLUDES = -I../src/include
LIBS = ../lib/libwren.a -lm

TARGET = game_test

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) main.c $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean