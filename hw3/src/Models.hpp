#ifndef __MODELS_HPP__
#define __MODELS_HPP__

#include <ostream>
#include <vector>

using Username = std::string;
using Password = std::string;
using Message = std::string;
using SessionId = int;
using IPAddress = std::string;
using Port = int;
using Version = int;
using Keywords = std::string;
using ViolationCount = int;

struct User {
	Username username;
	Password password;
	ViolationCount violations;

	bool operator==(const User &rhs) const {
		return username == rhs.username && password == rhs.password;
	}

	bool operator!=(const User &rhs) const {
		return !(*this == rhs);
	}

	friend std::ostream& operator<<(std::ostream &os, const User &user) {
		os << user.username << " " << user.password;
		return os;
	}
};

struct Chat {
	Username username;
	Message message;
};

struct Session {
	User user;
	int fd;
	IPAddress ip;
	Port port;
	Version version;

	bool operator==(const Session &rhs) const {
		return user == rhs.user && ip == rhs.ip && port == rhs.port && version == rhs.version;
	}

	bool operator!=(const Session &rhs) const {
		return !(*this == rhs);
	}

	friend std::ostream& operator<<(std::ostream &os, const Session &session) {
		os << session.ip << ":" << session.port << " " << session.user << ", v" << session.version;
		return os;
	}
};

#endif
