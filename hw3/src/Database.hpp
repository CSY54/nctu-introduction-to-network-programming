#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__

#include <vector>
#include <map>
#include <algorithm>
#include <locale>

#include "Models.hpp"
#include "utils.hpp"

const int VIOLATION_LIMIT = 3;

class Database {
private:
	std::map<Username, User> users;
	std::map<SessionId, Session> sessions;
	std::vector<Chat> chat;
	std::vector<Keywords> blacklist = {
		"how",
		"you",
		"or",
		"pek0",
		"tea",
		"ha",
		"kon",
		"pain",
		"Starburst Stream",
	};

public:
	Database();

	// users
	void create_user(Username, Password);
	std::vector<User> get_users();
	User get_user_by_username(Username username);
	int user_violate(Username username);
	int get_fd_by_ip_port(std::string, int);

	// sessions
	std::vector<Session> get_sessions();
	void set_session(SessionId, Session);
	Session get_session(SessionId);
	void clear_session(SessionId);
	void logout_user(SessionId);
	bool already_login(User);

	// blacklist
	std::vector<Keywords> get_blacklist();

	// chat
	void create_chat(std::string, std::string);
	std::vector<Chat> get_chat();
};

#endif
