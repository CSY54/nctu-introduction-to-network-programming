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
	int tcp_fd;
	int udp_fd;
	int epollfd;
	struct epoll_event events[MAX_EVENTS];
	Controller controller;

	void epoll_add_event(int);
	void setup_tcp(std::string, std::string);
	void setup_udp(std::string, std::string);

	void accept_connection(int);
	void tcp_handler(int);
	void udp_handler(int);

public:
	Server(std::string, std::string);

	void interactive();
};

#endif
