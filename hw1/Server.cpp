#include "Server.hpp"

namespace hw1 {

Server::Server(std::string ip, std::string port) {
	struct addrinfo hints;
	struct addrinfo *sock_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &sock_info) < 0) {
		handle_error("getaddrinfo");
	}

	sockfd = socket(sock_info->ai_family, sock_info->ai_socktype, sock_info->ai_protocol);

	if (sockfd < 0) {
		handle_error("socket");
	}

	if (bind(sockfd, sock_info->ai_addr, sock_info->ai_addrlen) < 0) {
		handle_error("bind");
	}

	if (listen(sockfd, MAX_CONNECTION) < 0) {
		handle_error("listen");
	}

}

void Server::interactive() {
	int clientfd;
	struct sockaddr_in peer;
	socklen_t peer_addr_size;

	while (true) {
		clientfd = accept(sockfd, (struct sockaddr*)&peer, &peer_addr_size);
		printf("clientfd: %d\n", clientfd);

		if (clientfd < 0) {
			handle_error("accept");
		}
		
		std::thread t(&Server::client_handler, this, clientfd); t.detach();
	}
}

void Server::client_handler(int clientfd) {
	if (send_banner(clientfd) < 0) {
		return;
	}

	char buffer[BUFFSIZE];
	while (true) {
		if (send_prompt(clientfd) < 0) {
			break;
		}

		memset(buffer, 0, sizeof(buffer));
		if (read(clientfd, buffer, sizeof(buffer)) <= 0) {
			shutdown(clientfd, SHUT_RDWR);
			close(clientfd);
			break;
		}

		auto [cmd, args] = commandize(std::string(buffer));

		if (command_handler.count(cmd)) {
			command_handler[cmd](clientfd, args);
		}
	}
}

int Server::send(int clientfd, std::string message, bool newline) {
	message = message + (newline ? "\n" : "");
	return write(clientfd, message.c_str(), message.size());
}

int Server::send_banner(int clientfd) {
	return send(
		clientfd,
		"********************************\n"
		"** Welcome to the BBS server. **\n"
		"********************************"
	);
}

int Server::send_prompt(int clientfd) {
	return send(
		clientfd,
		"% ",
		false
	);
}

std::pair<
	std::string,
	std::vector<std::string>
> Server::commandize(std::string s) {
	std::vector<std::string> res;
	unsigned int idx = 0;

	// strip newline
	if (!s.empty() && s[s.length() - 1] == '\n') {
		s.erase(s.length() - 1);
	}

	s += " ";
	while (idx < s.size()) {
		if (s[idx] == '"') {
			std::size_t nxt = s.find('"', idx + 1);
			if (nxt == std::string::npos) {
				break;
			}
			std::string tok = s.substr(idx + 1, nxt - idx - 1);
			res.emplace_back(tok);
			idx = nxt + 1;
		} else {
			std::size_t nxt = s.find(' ', idx + 1);
			if (nxt == std::string::npos) {
				break;
			}
			std::string tok = s.substr(idx, nxt - idx);
			res.emplace_back(tok);
			idx = nxt + 1;
		}
	}

	return {
		res[0],
		std::vector<std::string>(res.begin() + 1, res.end())
	};
}

}
