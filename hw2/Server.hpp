#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Controller.hpp"
#include "Database.hpp"
#include "utils.hpp"

const int BACKLOG = 10;
const int MAX_EVENTS = 50;
const int TIMEOUT = 1000;

class Server {
private:
	int serverfd;
	int epollfd;
	struct epoll_event events[MAX_EVENTS];
	Controller controller;

public:
	Server(std::string, std::string);

	void interactive();
	void accept_connection();
	void client_handler(int);
};

#endif
