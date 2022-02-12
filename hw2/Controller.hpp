#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

#include <map>
#include <sstream>

#include "Argument.hpp"
#include "Database.hpp"
#include "utils.hpp"

class Controller;

typedef void (Controller::*hdlr_t)(int, Argument);

class Controller {
private:
	void echo(int, Argument);
	void register_user(int, Argument);
	void login(int, Argument);
	void logout(int, Argument);
	void exit(int, Argument);
	void create_board(int, Argument);
	void create_post(int, Argument);
	void list_board(int, Argument);
	void list_all_post(int, Argument);
	void list_post(int, Argument);
	void read(int, Argument);
	void delete_post(int, Argument);
	void update_post(int, Argument);
	void comment(int, Argument);
	
	std::map<std::string, hdlr_t> commands;

	Database db;

public:
	Controller();
	void dispatch(int, std::string, Argument);

	void handle_disconnection(int);
};

#endif
