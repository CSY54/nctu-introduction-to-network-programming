CC=g++
CFLAGS=-std=c++17 -O2
DEBUG=-g -Wall -Wextra -Wshadow
OUTPUT=../test/hw3

HEADERS=Argument.hpp Controller.hpp Database.hpp Models.hpp Server.hpp utils.hpp
SOURCES=Argument.cpp Controller.cpp Database.cpp Models.hpp Server.cpp utils.cpp

all: $(HEADERS) $(SOURCES) hw3.cpp
	$(CC) $(CFLAGS) $(SOURCES) hw3.cpp -o $(OUTPUT)

debug: $(HEADERS) $(SOURCES) hw3.cpp
	$(CC) $(CFLAGS) $(DEBUG) $(SOURCES) hw3.cpp -o $(OUTPUT)

zip: $(HEADERS) $(SOURCES) hw3.cpp
	mkdir $(ID)_hw3
	cp $(HEADERS) $(SOURCES) hw3.cpp Makefile $(ID)_hw3
	zip -r ../$(ID)_hw3.zip $(ID)_hw3
	rm -r $(ID)_hw3
