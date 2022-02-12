#include "Server.hpp"

Server::Server(std::string ip, std::string port) {
	setup_tcp(ip, port);
	setup_udp(ip, port);

	epollfd = epoll_create(MAX_EVENTS);
	epoll_add_event(tcp_fd);
	epoll_add_event(udp_fd);
}

void Server::epoll_add_event(int fd) {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0)  {
		handle_error("epoll_ctl");
	}
}

void Server::setup_tcp(std::string ip, std::string port) {
	struct addrinfo hints;
	struct addrinfo *sock_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &sock_info) < 0) {
		handle_error("getaddrinfo");
	}

	tcp_fd = socket(sock_info->ai_family, sock_info->ai_socktype, sock_info->ai_protocol);
	debug("tcp_fd:", tcp_fd);

	if (tcp_fd < 0) {
		handle_error("socket");
	}

	const int reuse = 1;
	if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		handle_error("setsockopt");
	}

	if (bind(tcp_fd, sock_info->ai_addr, sock_info->ai_addrlen) < 0) {
		handle_error("bind");
	}

	if (listen(tcp_fd, BACKLOG) < 0) {
		handle_error("listen");
	}
}

void Server::setup_udp(std::string ip, std::string port) {
	struct addrinfo hints;
	struct addrinfo *sock_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &sock_info) < 0) {
		handle_error("getaddrinfo");
	}

	udp_fd = socket(sock_info->ai_family, sock_info->ai_socktype, sock_info->ai_protocol);
	debug("udp_fd:", udp_fd);

	if (udp_fd < 0) {
		handle_error("socket");
	}

	if (bind(udp_fd, sock_info->ai_addr, sock_info->ai_addrlen) < 0) {
		handle_error("bind");
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
				int clientfd = events[i].data.fd;
				debug("FD:", (int)events[i].data.fd, "EPOLLERR | EPOLLHUP");
				controller.handle_disconnection(clientfd);
				close_connection(clientfd);
			} else if (!(events[i].events & EPOLLIN)) {
				handle_error("epoll event error");
			}

			int eventfd = events[i].data.fd;
			if (eventfd == tcp_fd) {  // new TCP event
				info("New TCP event");
				accept_connection(eventfd);
			} else if (eventfd == udp_fd) {  // new UDP event
				info("New UDP event");
				udp_handler(eventfd);
			} else {  // an established TCP connection
				tcp_handler(eventfd);
			}
		}
	}
}

void Server::accept_connection(int serverfd) {
	sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(sockaddr_in);
	int clientfd = accept(serverfd, (sockaddr*)&client_addr, &client_addr_size);
	if (clientfd < 0) {
		handle_error("accept");
	}

	info("[+] New TCP:", clientfd);

	controller.handle_new_connection(
		clientfd, 
		std::string(inet_ntoa(client_addr.sin_addr))
	);

	epoll_add_event(clientfd);

	send_banner(clientfd);
	send_prompt(clientfd);
}

void Server::tcp_handler(int clientfd) {
	char buffer[MAX_BUFFER];
	memset(buffer, 0, sizeof(buffer));
	if (read(clientfd, buffer, sizeof(buffer)) < 0) {
		return;
	}

	auto [command, argument] = commandize(std::string(buffer));

	controller.dispatch(clientfd, command, argument);

	send_prompt(clientfd);
}

void Server::udp_handler(int clientfd) {
	sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	char buffer[MAX_BUFFER];
	int n;
	memset(buffer, 0, sizeof(buffer));
	if ((n = recvfrom(clientfd, buffer, MAX_BUFFER - 1, 0, (sockaddr*)&client_addr, &client_addr_len)) < 0) {
		return;
	}

	std::string ip(inet_ntoa(client_addr.sin_addr));
	int port = ntohs(client_addr.sin_port);

	debug("udp_handler", ip, port);

	controller.chat(ip, port, std::string(buffer, n));
}
