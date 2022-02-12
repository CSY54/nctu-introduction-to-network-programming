#include <iostream>

#include "Server.hpp"

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "server usage: ./hw2 [port number]" << std::endl;
		exit(EXIT_FAILURE);
	}

	Server server("127.0.0.1", argv[1]);
	server.interactive();

	return 0;
}
