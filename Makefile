LIBS = -lm -lcrypto -lgmp -lgmpxx
CXX = g++
CXXFLAGS = -O2 -Wall -g

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = build/client.o build/group.o build/hash.o build/random.o build/server.o build/binary_ros.o build/signature.o build/utils.o

HEADERS = $(wildcard *.h)

# remove built-in rule
%: %.cpp

build/%.o: src/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

%: build/%.o $(OBJECTS)
	$(CXX) $(OBJECTS) $< -Wall $(LIBS) -o $@

clean:
	-rm -f build/*
	-rm -f main
	-rm -f benchmark
	-rm -f test_bs
