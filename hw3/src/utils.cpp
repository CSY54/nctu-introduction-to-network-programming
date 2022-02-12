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

std::string _replace_all(std::string s, std::string from, std::string to) {
	size_t idx = 0;
	while ((idx = s.find(from)) != std::string::npos) {
		s.replace(idx, from.length(), to);
		idx += to.length();
	}
	return s;
}

std::string filter(std::vector<std::string> keywords, std::string s) {
	std::string res = s;
	for (auto &keyword : keywords) {
		res = _replace_all(res, keyword, std::string(keyword.length(), '*'));
	}
	return res;
}

std::string base64_encode(std::string s) {
	std::string res = "";
	int pad = s.length() % 3 == 0 ? 0 : 3 - s.length() % 3;

	s += std::string(pad, '\x00');

	for (int i = 0; i < s.length(); i += 3) {
		res += BASE64_TABLE[(s[i] & 0xfc) >> 2];
		res += BASE64_TABLE[((s[i] & 0x03) << 4) | ((s[i + 1] & 0xf0) >> 4)];
		res += BASE64_TABLE[((s[i + 1] & 0x0f) << 2) | ((s[i + 2] & 0xc0) >> 6)];
		res += BASE64_TABLE[s[i + 2] & 0x3f];
	}

	if (res.length() >= pad) {
		for (int i = 0; i < pad; i++) {
			res[res.length() - i - 1] = '=';
		}
	}

	return res;
}

std::string base64_decode(std::string s) {
	std::string res = "";

	std::vector<int> dtable(256, 0x80);

	for (int i = 0; i < (int)BASE64_TABLE.length(); i++) {
		dtable[BASE64_TABLE[i]] = i;
	}

	if (s.length() % 4) {
		return "";
	}

	int pad = 0;
	for (int i = 0; s.length() - i - 1 >= 0 && s[s.length() - i - 1] == '='; i++) {
		pad++;
		s[s.length() - i - 1] = 'A';
	}

	if (pad > 2) {
		return "";
	}

	// all(ch in BASE64_TABLE for ch in s)
	for (auto &ch : s) {
		if (dtable[ch] == 0x80) {
			return "";
		}
	}

	for (int i = 0; i < s.length(); i += 4) {
		res += ((dtable[s[i]] & 0x3f) << 2) | ((dtable[s[i + 1]] & 0x30) >> 4);
		res += ((dtable[s[i + 1]] & 0x0f) << 4) | ((dtable[s[i + 2]] & 0x3c) >> 2);
		res += ((dtable[s[i + 2]] & 0x03) << 6) | (dtable[s[i + 3]] & 0x3f);
	}

	while (pad--) {
		res.pop_back();
	}

	return res;
}
