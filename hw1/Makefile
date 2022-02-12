CC=g++
CFLAGS=-std=c++17 -O2 -Wall -Wextra -Wshadow
DEBUG=-DDEBUG
LNK=-lpthread

hw1: main.cpp Server.cpp Server.hpp
	$(CC) $(CFLAGS) Server.cpp main.cpp -o hw1 $(LNK)

hw1-debug: main.cpp Server.cpp Server.hpp
	$(CC) $(CFLAGS) $(DEBUG) main.cpp -o hw1 $(LNK)

all: hw1

zip: main.cpp Server.cpp Server.hpp Makefile
	mkdir $(ID)_hw1
	cp main.cpp Server.cpp Server.hpp Makefile $(ID)_hw1
	zip -r $(ID)_hw1.zip $(ID)_hw1
	rm -r $(ID)_hw1

.PHONY: clean
clean: hw1
	rm hw1
