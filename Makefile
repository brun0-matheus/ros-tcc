LIBS = -lm -lcrypto -lgmp
CC = gcc
CFLAGS = -O2 -Wall 

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = build/client.o build/group.o build/hash.o build/random.o build/server.o
HEADERS = $(wildcard *.h)

build/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJECTS) build/main.o
	$(CC) $(OBJECTS) build/main.o -Wall $(LIBS) -o main

benchmark: $(OBJECTS) build/benchmark.o
	$(CC) $(OBJECTS) build/benchmark.o -Wall $(LIBS) -o benchmark

clean:
	-rm -f build/*
