#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

#include <map>
#include <sstream>
#include <cassert>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Argument.hpp"
#include "Database.hpp"
#include "utils.hpp"

class Controller;

typedef void (Controller::*hdlr_t)(int, Argument);

class Controller {
private:
	struct ChatPayload {
		uint8_t flag;
		uint8_t version;
		std::string name;
		std::string message;

		bool operator==(const ChatPayload &rhs) const {
			return flag == rhs.flag && version == rhs.version && name == rhs.name && message == rhs.message;
		}

		std::string serialize(int version) const {
			std::string res = "\x01";

			if (version == 1) {
				res += "\x01";
				res += (name.length() >> 8) & 0xff;
				res += name.length() & 0xff;
				res += name;
				res += (message.length() >> 8) & 0xff;
				res += message.length() & 0xff;
				res += message;
			} else {
				res += "\x02";
				res += base64_encode(name);
				res += "\n";
				res += base64_encode(message);
				res += "\n";
			}

			return res;
		}

		friend std::ostream& operator<<(std::ostream &os, const ChatPayload &payload) {
			os << "flag: " << (int)payload.flag << std::endl
				<< "version: " << (int)payload.version << std::endl
				<< "name: " << payload.name << std::endl
				<< "message: " << payload.message << std::endl;
			return os;
		}
	};

	ChatPayload parse_chat_payload(std::string);

	// commands
	void echo(int, Argument);
	void register_user(int, Argument);
	void login(int, Argument);
	void logout(int, Argument);
	void exit(int, Argument);
	void enter_chat_room(int, Argument);

	std::map<std::string, hdlr_t> commands;

	Database db;

public:
	Controller();
	void dispatch(int, std::string, Argument);
	void chat(std::string, int, std::string);

	void handle_disconnection(int);
	void handle_new_connection(int, std::string);
};

#endif
