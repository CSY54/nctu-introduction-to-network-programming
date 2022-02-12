#include "Controller.hpp"

Controller::Controller() {
	commands["echo"] = &Controller::echo;
	commands["register"] = &Controller::register_user;
	commands["login"] = &Controller::login;
	commands["logout"] = &Controller::logout;
	commands["exit"] = &Controller::exit;
	commands["create-board"] = &Controller::create_board;
	commands["create-post"] = &Controller::create_post;
	commands["list-board"] = &Controller::list_board;
	commands["list-all-post"] = &Controller::list_all_post;
	commands["list-post"] = &Controller::list_post;
	commands["read"] = &Controller::read;
	commands["delete-post"] = &Controller::delete_post;
	commands["update-post"] = &Controller::update_post;
	commands["comment"] = &Controller::comment;
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

	if (guard(
		db.get_session(clientfd) == User{},
		clientfd,
		"Please logout first."
	)) {
		return;
	}

	Username username = arg.args[0];
	Password password = arg.args[1];

	User user = db.get_user_by_username(username);
	if (guard(
		!db.already_login(user),
		clientfd,
		"User already login."
	)) {
		return;
	}

	if (guard(
		user != User{} && user.password == password,
		clientfd,
		"Login failed."
	)) {
		return;
	}

	db.set_session(clientfd, user);
	sendline(clientfd, "Welcome, " + username + ".");
}

void Controller::logout(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 0,
		clientfd,
		"Usage: logout"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	db.clear_session(clientfd);
	sendline(clientfd, "Bye, " + user.username + ".");
}

void Controller::exit(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 0,
		clientfd,
		"Usage: exit"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (user != User{}) {
		db.clear_session(clientfd);
		sendline(clientfd, "Bye, " + user.username + ".");
	}

	close_connection(clientfd);
}

void Controller::create_board(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 1 && arg.kwargs.size() == 0,
		clientfd,
		"Usage: create-board <name>"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	BoardName board_name = arg.args[0];
	Board board = db.get_board_by_name(board_name);
	if (guard(
		board == Board{},
		clientfd,
		"Board already exists."
	)) {
		return;
	}

	db.create_board(board_name, user.username);
	sendline(clientfd, "Create board successfully.");
}

void Controller::create_post(int clientfd, Argument arg) {
	if (guard(
		(
			arg.args.size() == 1
			&& arg.kwargs.size() == 2
			&& arg.kwargs.count("title")
			&& arg.kwargs.count("content")
		),
		clientfd,
		"Usage: create-post <board-name> --title <title> --content <content>"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	BoardName board_name = arg.args[0];
	Board board = db.get_board_by_name(board_name);
	debug(board);
	if (guard(
		board != Board{},
		clientfd,
		"Board does not exist."
	)) {
		return;
	}

	PostTitle title = arg.kwargs["title"];
	PostContent content = replace_all(arg.kwargs["content"], "<br>", "\n");
	db.create_post(title, content, user.username, board_name);
	sendline(clientfd, "Create post successfully.");
}

void Controller::list_board(int clientfd, Argument arg) {
	std::vector<Board> boards = db.get_boards();

	std::stringstream ss;
	ss << "Index\tName\tModerator" << std::endl;
	for (auto &board : boards) {
		ss << board.id
			<< "\t" << board.name
			<< "\t" << board.moderator
			<< std::endl;
	}
	sendline(clientfd, ss.str());
}

void Controller::list_all_post(int clientfd, Argument arg) {
	if (guard(
		arg.argc == 0,
		clientfd,
		"Usage: list-all-post"
	)) {
		return;
	}

	std::vector<Post> posts = db.get_posts();

	std::stringstream ss;
	ss << "S/N\tTitle\tAuthor\tDate" << std::endl;
	for (auto &post : posts) {
		ss << post.id
			<< "\t" << post.title
			<< "\t" << post.author
			<< "\t" << post.date
			<< std::endl;
	}
	sendline(clientfd, ss.str());
}

void Controller::list_post(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 1 && arg.kwargs.size() == 0,
		clientfd,
		"Usage: list-post <board-name>"
	)) {
		return;
	}

	BoardName board_name = arg.args[0];
	Board board = db.get_board_by_name(board_name);
	if (guard(
		board != Board{},
		clientfd,
		"Board does not exist."
	)) {
		return;
	}

	std::vector<Post> posts = db.get_posts_by_board_name(board_name);

	std::stringstream ss;
	ss << "S/N\tTitle\tAuthor\tDate" << std::endl;
	for (auto &post : posts) {
		ss << post.id
			<< "\t" << post.title
			<< "\t" << post.author
			<< "\t" << post.date
			<< std::endl;
	}
	sendline(clientfd, ss.str());
}

void Controller::read(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 1 && arg.kwargs.size() == 0 && is_int(arg.args[0]),
		clientfd,
		"Usage: read <post-S/N>"
	)) {
		return;
	}

	PostId post_id = to_int(arg.args[0]);
	Post post = db.get_post_by_id(post_id);
	if (guard(
		post != Post{},
		clientfd,
		"Post does not exist."
	)) {
		return;
	}

	std::stringstream ss;
	ss << post;
	sendline(clientfd, ss.str());
}

void Controller::delete_post(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 1 && arg.kwargs.size() == 0 && is_int(arg.args[0]),
		clientfd,
		"Usage: delete-post <post-S/N>"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	PostId post_id = to_int(arg.args[0]);
	Post post = db.get_post_by_id(post_id);
	if (guard(
		post != Post{},
		clientfd,
		"Post does not exist."
	)) {
		return;
	}

	if (guard(
		post.author == user.username,
		clientfd,
		"Not the post owner."
	)) {
		return;
	}

	db.delete_post_by_id(post_id);
	sendline(clientfd, "Delete successfully.");
}

void Controller::update_post(int clientfd, Argument arg) {
	if (guard(
		(
			arg.args.size() == 1
			&& arg.kwargs.size() == 1
			&& is_int(arg.args[0])
			&& (arg.kwargs.count("title") || arg.kwargs.count("content"))
		),
		clientfd,
		"Usage: update-post <post-S/N> --title/content <new>"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please login first."
	)) {
		return;
	}

	PostId post_id = to_int(arg.args[0]);
	Post post = db.get_post_by_id(post_id);
	if (guard(
		post != Post{},
		clientfd,
		"Post does not exist."
	)) {
		return;
	}

	if (guard(
		post.author == user.username,
		clientfd,
		"Not the post owner."
	)) {
		return;
	}

	if (arg.kwargs.count("title")) {
		db.update_post_title_by_id(post_id, arg.kwargs["title"]);
	} else {
		db.update_post_content_by_id(
			post_id,
			replace_all(arg.kwargs["content"], "<br>", "\n")
		);
	}
	sendline(clientfd, "Update successfully.");
}

void Controller::comment(int clientfd, Argument arg) {
	if (guard(
		arg.args.size() == 2 && arg.kwargs.size() == 0 && is_int(arg.args[0]),
		clientfd,
		"Usage: comment <post-S/N> <comment>"
	)) {
		return;
	}

	User user = db.get_session(clientfd);
	if (guard(
		user != User{},
		clientfd,
		"Please log in first."
	)) {
		return;
	}

	PostId post_id = to_int(arg.args[0]);
	Post post = db.get_post_by_id(post_id);
	if (guard(
		post != Post{},
		clientfd,
		"Post does not exist."
	)) {
		return;
	}

	CommentContent comment = arg.args[1];
	db.create_comment(post_id, user.username, comment);
	sendline(clientfd, "Comment successfully.");
}
