#include "Database.hpp"

Database::Database() {}

void Database::create_user(Username username, Password password) {
	users[username] = User{username, password};
}

std::vector<User> Database::get_users() {
	std::vector<User> res;
	transform(
		users.begin(),
		users.end(),
		back_inserter(res),
		[](std::pair<Username, User> user) {
			return user.second;
		}
	);

	return res;
}

User Database::get_user_by_username(Username username) {
	return users.count(username) ? users[username] : User{};
}

int Database::user_violate(Username username) {
	users[username].violations += 1;
	return users[username].violations;
}

int Database::get_fd_by_ip_port(std::string ip, int port) {
	for (auto &[_, session] : sessions) {
		debug(session.ip, session.port, session.fd);
	}
	for (auto &[_, session] : sessions) {
		if (session.ip == ip && session.port == port) {
			return session.fd;
		}
	}
	return -1;
}

std::vector<Session> Database::get_sessions() {
	std::vector<Session> res;
	std::transform(
		sessions.begin(),
		sessions.end(),
		std::back_inserter(res),
		[](std::pair<SessionId, Session> session) {
			return session.second;
		}
	);

	return res;
}

void Database::set_session(SessionId id, Session session) {
	if (session.user != User{}) {
		sessions[id].user = session.user;
	}

	if (session.fd != 0) {
		sessions[id].fd = session.fd;
	}

	if (session.ip != "") {
		sessions[id].ip = session.ip;
	}

	if (session.port != 0) {
		sessions[id].port = session.port;
	}

	if (session.version != 0) {
		sessions[id].version = session.version;
	}
}

Session Database::get_session(SessionId id) {
	return sessions.count(id) ? sessions[id] : Session{};
}

void Database::clear_session(SessionId id) {
	sessions.erase(id);
}

void Database::logout_user(SessionId id) {
	sessions[id].user = User{};
	sessions[id].port = 0;
	sessions[id].version = 0;
}

bool Database::already_login(User user) {
	for (auto &[_, session] : sessions) {
		if (session.user == user) {
			return true;
		}
	}
	return false;
}

std::vector<Keywords> Database::get_blacklist() {
	return blacklist;
}

void Database::create_chat(std::string username, std::string message) {
	chat.emplace_back(Chat{username, message});
}

std::vector<Chat> Database::get_chat() {
	return chat;
}
