#include "Controller.hpp"

Controller::Controller() {
	commands["echo"] = &Controller::echo;
	commands["register"] = &Controller::register_user;
	commands["login"] = &Controller::login;
	commands["logout"] = &Controller::logout;
	commands["exit"] = &Controller::exit;
	commands["enter-chat-room"] = &Controller::enter_chat_room;
}

void Controller::dispatch(int clientfd, std::string command, Argument arg) {
	info_fd(clientfd, command, arg);
	if (commands.count(command)) {
		(this->*commands[command])(clientfd, arg);
	}
}

void Controller::handle_disconnection(int clientfd) {
	db.clear_session(clientfd);
}

void Controller::handle_new_connection(int clientfd, std::string ip) {
	db.set_session(clientfd, Session{.fd = clientfd, .ip = ip});
}

void Controller::echo(int clientfd, Argument arg) {
	sendline(clientfd, arg.args[0]);
}

void Controller::register_user(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 2 && arg.kwargs.size() == 0,
		clientfd,
		"Usage: register <username> <password>"
	)) {
		return;
	}

	Username username = arg.args[0];
	Password password = arg.args[1];

	if (guard(
		db.get_user_by_username(username) == User{},
		clientfd,
		"Username is already used."
	)) {
		return;
	}

	db.create_user(username, password);
	sendline(clientfd, "Register successfully.");
}

void Controller::login(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 2 && arg.kwargs.size() == 0,
		clientfd,
		"Usage: login <username> <password>"
	)) {
		return;
	}

	Session session = db.get_session(clientfd);
	if (guard(
		session.user == User{},
		clientfd,
		"Please logout first."
	)) {
		return;
	}

	Username username = arg.args[0];
	Password password = arg.args[1];

	User user = db.get_user_by_username(username);
	if (guard(
		user != User{},
		clientfd,
		"Login failed."
	)) {
		return;
	}

	if (guard(
		user.violations < VIOLATION_LIMIT,
		clientfd,
		"We don't welcome " + user.username + "!"
	)) {
		return;
	}

	if (guard(
		user.password == password,
		clientfd,
		"Login failed."
	)) {
		return;
	}

	// if (guard(
	//     !db.already_login(user),
	//     clientfd,
	//     "User already login."
	// )) {
	//     return;
	// }
    //
	// db.set_session(clientfd, user);
	// sendline(clientfd, "Welcome, " + username + ".");

	if (!db.already_login(user)) {  // condition 2?
		db.set_session(clientfd, Session{.user = user});
		sendline(clientfd, "Welcome, " + user.username + ".");
	}
}

void Controller::logout(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 0,
		clientfd,
		"Usage: logout"
	)) {
		return;
	}

	Session session = db.get_session(clientfd);
	if (guard(
		session.user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	db.logout_user(clientfd);
	sendline(clientfd, "Bye, " + session.user.username + ".");
}

void Controller::exit(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 0,
		clientfd,
		"Usage: exit"
	)) {
		return;
	}

	Session session = db.get_session(clientfd);
	if (session.user != User{}) {
		db.clear_session(clientfd);
		sendline(clientfd, "Bye, " + session.user.username + ".");
	}

	close_connection(clientfd);
}

void Controller::enter_chat_room(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 2 && arg.args.size() == 2,
		clientfd,
		"Usage: enter-chat-room <port> <version>"
	)) {
		return;
	}

	if (guard(
		is_int(arg.args[0]) && to_int(arg.args[0]) > 0 && to_int(arg.args[0]) < 65536,
		clientfd,
		"Port " + arg.args[0] + " is not valid."
	)) {
		return;
	}

	if (guard(
		is_int(arg.args[1]) && (to_int(arg.args[1]) == 1 || to_int(arg.args[1]) == 2),
		clientfd,
		"Version " + arg.args[1] + " is not supported."
	)) {
		return;
	}

	Session session = db.get_session(clientfd);
	if (guard(
		session.user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	session.port = to_int(arg.args[0]);
	session.version = to_int(arg.args[1]);

	db.set_session(clientfd, session);

	std::stringstream ss;
	ss << "Welcome to public chat room." << std::endl
		<< "Port:" << session.port << std::endl
		<< "Version:" << session.version << std::endl;

	std::vector<Chat> chat_history = db.get_chat();
	for (auto &chat : chat_history) {
		ss << chat.username << ":" << chat.message << std::endl;
	}

	sendline(clientfd, ss.str());
}

void Controller::chat(std::string ip, int port, std::string packet) {
	Controller::ChatPayload chat_payload = parse_chat_payload(packet);
	if (chat_payload == ChatPayload{}) {
		return;
	}

	info(chat_payload);

	// TODO: check ip:port, username are matched
	int fd = db.get_fd_by_ip_port(ip, port);
	if (fd == -1) {
		return;
	}

	Session session = db.get_session(fd);
	debug(session.ip, "==", ip);
	debug(session.port, "==", port);
	debug(session.user.username, "==", chat_payload.name);
	if (
		session.ip != ip
		|| session.port != port
		|| session.user.username != chat_payload.name
	) {
		return;
	}

	std::string filtered_message = filter(db.get_blacklist(), chat_payload.message);
	if (filtered_message != chat_payload.message) {
		int violations = db.user_violate(chat_payload.name);
		if (violations >= VIOLATION_LIMIT) {
			logout(fd, Argument());
			// feed prompt manually...
			send_prompt(fd);
		}
	}

	chat_payload.message = filtered_message;

	db.create_chat(chat_payload.name, chat_payload.message);

	for (auto &session : db.get_sessions()) {
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0) {
			continue;
		}

		if (session.port == 0 || session.version == 0) {
			continue;
		}

		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(session.ip.c_str());
		addr.sin_port = htons(session.port);

		std::string payload = chat_payload.serialize(session.version);
		sendto(sockfd, payload.c_str(), payload.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
	}
}

Controller::ChatPayload Controller::parse_chat_payload(std::string payload) {
	if (payload.length() < 2) {
		return ChatPayload{};
	}

	uint8_t flag = payload[0];
	if (flag != 0x1) {
		return ChatPayload{};
	}

	uint8_t version = payload[1];
	if (version != 0x1 && version != 0x2) {
		return ChatPayload{};
	}

	if (version == 0x1) {
		if (payload.length() < 2 + 2) {
			return ChatPayload{};
		}
		int idx = 2;
		int name_length = payload[idx] << 8 | payload[idx + 1];
		idx += 2;

		if (payload.length() < 2 + 2 + name_length) {
			return ChatPayload{};
		}
		std::string name = payload.substr(idx, name_length);
		idx += name_length;

		if (payload.length() < 2 + 2 + name_length + 2) {
			return ChatPayload{};
		}
		int message_length = payload[idx] << 8 | payload[idx + 1];
		idx += 2;

		if (payload.length() < 2 + 2 + name_length + 2 + message_length) {
			return ChatPayload{};
		}
		std::string message = payload.substr(idx, message_length);
		idx += message_length;

		if (idx != payload.length()) {
			return ChatPayload{};
		}

		return ChatPayload{
			.flag = flag,
			.version = version,
			.name = name,
			.message = message
		};
	} else {
		payload = payload.substr(2, payload.length() - 2);
		size_t next_idx = payload.find("\n");
		if (next_idx == std::string::npos) {
			return ChatPayload{};
		}

		std::string encoded_name = payload.substr(0, next_idx);
		payload = payload.substr(next_idx + 1, payload.length() - next_idx - 1);

		next_idx = payload.find("\n");
		if (next_idx == std::string::npos){
			return ChatPayload{};
		}

		std::string encoded_message = payload.substr(0, next_idx);
		payload = payload.substr(next_idx + 1, payload.length() - next_idx - 1);

		if (payload != "") {
			return ChatPayload{};
		}

		std::string decoded_name = base64_decode(encoded_name);
		std::string decoded_message = base64_decode(encoded_message);

		if (decoded_name == "" || decoded_message == "") {
			return ChatPayload{};
		}

		return ChatPayload{
			.flag = flag,
			.version = version,
			.name = decoded_name,
			.message = decoded_message
		};
	}
}
