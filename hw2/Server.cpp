#include "Server.hpp"

Server::Server(std::string ip, std::string port) {
	struct addrinfo hints;
	struct addrinfo *sock_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &sock_info) < 0) {
		handle_error("getaddrinfo");
	}

	serverfd = socket(sock_info->ai_family, sock_info->ai_socktype, sock_info->ai_protocol);

	if (serverfd < 0) {
		handle_error("socket");
	}

	if (bind(serverfd, sock_info->ai_addr, sock_info->ai_addrlen) < 0) {
		handle_error("bind");
	}

	if (listen(serverfd, BACKLOG) < 0) {
		handle_error("listen");
	}

	epollfd = epoll_create(MAX_EVENTS);
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = serverfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) < 0)  {
		handle_error("epoll_ctl");
	}
}

void Server::interactive() {
	while (true) {
		int event_amount = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT);
		if (event_amount < 0) {
			handle_error("epoll_wait");
		} else if (event_amount == 0) {
			continue;
		}

		for (int i = 0; i < event_amount; i++) {
			if ((events[i].events & (EPOLLERR | EPOLLHUP))) {
				debug("EPOLLERR | EPOLLHUP");
				int clientfd = events[i].data.fd;
				debug("FD:", (int)events[i].data.fd);
				controller.handle_disconnection(clientfd);
				close_connection(clientfd);
			} else if (!(events[i].events & EPOLLIN)) {
				handle_error("epoll event error");
			}

			int eventfd = events[i].data.fd;
			if (eventfd == serverfd) {  // new event
				accept_connection();
			} else {
				client_handler(eventfd);
			}
		}
	}
}

void Server::accept_connection() {
	sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(sockaddr_in);
	int clientfd = accept(serverfd, (sockaddr*)&peer_addr, &peer_addr_size);
	if (clientfd < 0) {
		handle_error("accept");
	}

	info("[+] New connection:", clientfd);

	struct epoll_event event;
	event.data.fd = clientfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);

	send_banner(clientfd);
	send_prompt(clientfd);
}

void Server::client_handler(int clientfd) {
	char buffer[MAX_BUFFER];
	memset(buffer, 0, sizeof(buffer));
	if (read(clientfd, buffer, sizeof(buffer)) < 0) {
		return;
	}

	auto [command, argument] = commandize(std::string(buffer));

	controller.dispatch(clientfd, command, argument);

	send_prompt(clientfd);
}
