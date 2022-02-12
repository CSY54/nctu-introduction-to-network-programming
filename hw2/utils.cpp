#include "utils.hpp"

void close_connection(int clientfd) {
	if (close(clientfd) < 0) {
		handle_error(clientfd, "close");
	} else {
		info("[-] Connection closed:", clientfd);
	}
}

int _send(int clientfd, std::string message) {
	return write(clientfd, message.data(), message.size());
}

int sendline(int clientfd, std::string message) {
	if (message.size() > 0 && message.back() != '\n') {
		return _send(clientfd, message + std::string("\n"));
	} else {
		return _send(clientfd, message);
	}
}

std::string recvline(int clientfd) {
	char buffer[MAX_BUFFER];
	memset(buffer, 0, sizeof(buffer));
	if (read(clientfd, buffer, sizeof(buffer)) < 0) {
		return "";
	}

	return std::string(buffer);
}

int send_banner(int clientfd) {
	return sendline(
		clientfd,
		"********************************\n"
		"** Welcome to the BBS server. **\n"
		"********************************"
	);
}

int send_prompt(int clientfd) {
	return _send(clientfd, "% ");
}

// are keyword arguments always after positional argument?
// ask god
std::pair<std::string, Argument> commandize(std::string input) {
	// strip newline
	if (!input.empty() && input.back() == '\n') {
		input.pop_back();
	}

	input += " ";

	Argument argument;
	std::string command;
	int idx = 0;
	while (idx < input.size()) {
		if (input.substr(idx, 2) == "--") {  // keyword argument
			idx += 2;
			int len = 0;
			// keyword
			std::size_t nxt = input.find(' ', idx + 1);
			if (nxt == std::string::npos) {
				break;
			}
			std::string keyword = input.substr(idx, nxt - idx);
			idx = nxt + 1;

			// value
			len = 1;
			while (idx + len - 1 < input.size() && input.substr(idx + len - 1, 2) != "--") {
				len++;
			}
			std::string value = input.substr(idx, len - 2);
			idx += len - 1;

			argument.kwargs[keyword] = value;
		} else {  // command & positional argument
			// special case
			if (command == "comment" && argument.args.size() == 1) {
				argument.args.emplace_back(input.substr(idx, input.size() - idx));
				idx = input.size();
			} else {
				std::size_t nxt = input.find(' ', idx + 1);
				if (nxt == std::string::npos) {
					break;
				}
				std::string value = input.substr(idx, nxt - idx);
				idx = nxt + 1;

				if (command == "") {
					command = value;
				} else {
					argument.args.emplace_back(value);
				}
			}
		}
	}

	argument.argc = argument.args.size() + argument.kwargs.size();

	return {command, argument};
};

bool guard(bool condition, int clientfd, std::string msg) {
	if (!condition) {
		sendline(clientfd, msg);
	}

	return !condition;
}

bool is_int(std::string s) {
	try {
		std::stoi(s);
		return true;
	} catch (...) {
		return false;
	}
}

int to_int(std::string s) {
	return std::stoi(s);
}

std::string replace_all(std::string s, std::string from, std::string to) {
	size_t idx = 0;
	while ((idx = s.find(from)) != std::string::npos) {
		s.replace(idx, from.length(), to);
		idx += to.length();
	}
	return s;
}
