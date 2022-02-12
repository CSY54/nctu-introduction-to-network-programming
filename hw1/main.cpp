#include "Server.hpp"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: ./%s <port>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	hw1::Server server("127.0.0.1", argv[1]);

	server.interactive();

	return 0;
}
