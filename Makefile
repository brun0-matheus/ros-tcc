LIBS = -lm -lcrypto -lgmp
CC = gcc
CFLAGS = -O2 -Wall 

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = build/client.o build/group.o build/hash.o build/random.o build/server.o
HEADERS = $(wildcard *.h)

# remove built-in rule
%: %.c

build/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

%: build/%.o $(OBJECTS)
	$(CC) $(OBJECTS) $< -Wall $(LIBS) -o $@

clean:
	-rm -f build/*
	-rm -f main
	-rm -f benchmark
	-rm -f test_bs
