CC=g++
CFLAGS=-std=c++17 -O2
DEBUG=-g -Wall -Wextra -Wshadow
OUTPUT=hw2

HEADERS=Argument.hpp Controller.hpp Database.hpp Models.hpp Server.hpp utils.hpp
SOURCES=Argument.cpp Controller.cpp Database.cpp Models.hpp Server.cpp utils.cpp

all: $(HEADERS) $(SOURCES) hw2.cpp
	$(CC) $(CFLAGS) $(SOURCES) hw2.cpp -o $(OUTPUT)

debug: $(HEADERS) $(SOURCES) hw2.cpp
	$(CC) $(CFLAGS) $(DEBUG) $(SOURCES) hw2.cpp -o $(OUTPUT)

zip: $(HEADERS) $(SOURCES) hw2.cpp
	mkdir $(ID)_hw2
	cp $(HEADERS) $(SOURCES) hw2.cpp Makefile $(ID)_hw2
	zip -r $(ID)_hw2.zip $(ID)_hw2
	rm -r $(ID)_hw2
