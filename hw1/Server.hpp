#ifndef _SERVER_HPP_
#define _SERVER_HPP_

// lol, that's alot
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  /* inet_pton */
#include <netdb.h>  /* addrinfo */
#include <unistd.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <thread>
#include <algorithm>
#include <cstring>
#include <functional>
#include <mutex>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

namespace hw1 {

const int BUFFSIZE = 1024;

class Server {
private:
	typedef std::string Username;
	typedef std::string Password;
	typedef std::string Message;

	struct User {
		Username username;
		Password password;
	};

	struct Inbox {
		Username from;
		Message message;
	};

	// server socket fd
	int sockfd;

	// username -> user_id
	std::map<Username, int> user_id;
	// user_id -> username
	std::vector<User> users;
	// user_id -> inbox
	std::map<int, std::vector<Inbox>> inbox;

	// clientfd -> username
	std::map<int, Username> session;

	std::mutex mut_user_id;
	std::mutex mut_users;
	std::mutex mut_inbox;
	std::mutex mut_session;

	// bruh, it shouldn't be here
	std::map<
		std::string,
		std::function<void(int, std::vector<std::string>)>
	> command_handler = {
		{
			"register",
			[&](int clientfd, std::vector<std::string> argv) {
				const std::lock_guard<std::mutex> lock_user_id(mut_user_id);
				const std::lock_guard<std::mutex> lock_users(mut_users);

				if (argv.size() < 2) {
					send(
						clientfd,
						"Usage: register <username> <password>"
					);
					return;
				}

				Username username = argv[0];
				Password password = argv[1];

				if (user_id.count(username)) {
					send(
						clientfd,
						"Username is already used."
					);
					return;
				}

				user_id[username] = users.size();
				users.emplace_back(User{username, password});

				send(
					clientfd,
					"Register successfully."
				);
			}
		},
		{
			"login",
			[&](int clientfd, std::vector<std::string> argv) {
				const std::lock_guard<std::mutex> lock_user_id(mut_user_id);
				const std::lock_guard<std::mutex> lock_users(mut_users);
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (argv.size() < 2) {
					send(
						clientfd,
						"Usage: login <username> <password>"
					);
					return;
				}

				if (session[clientfd] != "") {
					send(
						clientfd,
						"Please logout first."
					);
					return;
				}

				Username username = argv[0];
				Password password = argv[1];

				if (!user_id.count(username) || users[user_id[username]].password != password) {
					send(
						clientfd,
						"Login failed."
					);
					return;
				}

				session[clientfd] = username;
				send(
					clientfd,
					"Welcome, " + username + "."
				);
			}
		},
		{
			"logout",
			[&](int clientfd, std::vector<std::string> /* argv */) {
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (session.count(clientfd)) {
					Username username = session[clientfd];
					session.erase(clientfd);
					send(
						clientfd,
						"Bye, " + username + "."
					);
					return;
				}

				send(
					clientfd,
					"Please login first."
				);
			}
		},
		{
			"whoami",
			[&](int clientfd, std::vector<std::string> /* argv */) {
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (session.count(clientfd)) {
					send(
						clientfd,
						session[clientfd]
					);
					return;
				}

				send(
					clientfd,
					"Please login first."
				);
			}
		},
		{
			"list-user",
			[&](int clientfd, std::vector<std::string> /* argv */) {
				const std::lock_guard<std::mutex> lock_users(mut_users);

				std::vector<std::string> v;
				transform(
					users.begin(),
					users.end(),
					std::back_inserter(v),
					[](const User& u) {
						return u.username;
					}
				);

				sort(v.begin(), v.end());

				for (auto &i : v) {
					send(clientfd, i);
				}
			}
		},
		{
			"exit",
			[&](int clientfd, std::vector<std::string> /* argv */) {
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (session.count(clientfd)) {
					send(
						clientfd,
						"Bye, " + session[clientfd] + "."
					);
					session.erase(clientfd);
				}

				if (shutdown(clientfd, SHUT_RDWR) < 0) {
					handle_error("shutdown");
					return;
				}

				if (close(clientfd) < 0) {
					handle_error("close");
					return;
				}
			}
		},
		{
			"send",
			[&](int clientfd, std::vector<std::string> argv) {
				const std::lock_guard<std::mutex> lock_user_id(mut_user_id);
				const std::lock_guard<std::mutex> lock_inbox(mut_inbox);
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (argv.size() < 2) {
					send(
						clientfd,
						"Usage: send <username> <message>"
					);
					return;
				}

				if (!session.count(clientfd)) {
					send(
						clientfd,
						"Please login first."
					);
					return;
				}

				Username username = argv[0];
				Message message = argv[1];

				if (!user_id.count(username)) {
					send(
						clientfd,
						"User not existed."
					);
					return;
				}

				inbox[user_id[username]].emplace_back(Inbox{session[clientfd], message});
			}
		},
		{
			"list-msg",
			[&](int clientfd, std::vector<std::string> /* argv */) {
				const std::lock_guard<std::mutex> lock_user_id(mut_user_id);
				const std::lock_guard<std::mutex> lock_inbox(mut_inbox);
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (!session.count(clientfd)) {
					send(
						clientfd,
						"Please login first."
					);
					return;
				}

				Username username = session[clientfd];
				std::vector<Inbox> in = inbox[user_id[username]];

				if (in.size() == 0) {
					send(
						clientfd,
						"Your message box is empty."
					);
					return;
				}

				std::map<Username, int> count;
				for (auto &i : in) {
					count[i.from]++;
				}

				for (auto &[from, cnt] : count) {
					send(
						clientfd,
						std::to_string(cnt) + " message from " + from + "."
					);
				}
			}
		},
		{
			"receive",
			[&](int clientfd, std::vector<std::string> argv) {
				const std::lock_guard<std::mutex> lock_user_id(mut_user_id);
				const std::lock_guard<std::mutex> lock_inbox(mut_inbox);
				const std::lock_guard<std::mutex> lock_session(mut_session);

				if (!session.count(clientfd)) {
					send(
						clientfd,
						"Please login first."
					);
					return;
				}

				if (argv.size() < 1) {
					send(
						clientfd,
						"Usage: receive <username>"
					);
					return;
				}

				Username username = argv[0];

				if (!user_id.count(username)) {
					send(
						clientfd,
						"User not existed."
					);
					return;
				}

				std::vector<Inbox> in = inbox[user_id[session[clientfd]]];

				if (in.size() == 0) {
					send(
						clientfd,
						"Your message box is empty."
					);
					return;
				}

				for (unsigned int i = 0; i < in.size(); i++) {
					if (in[i].from == username) {
						send(
							clientfd,
							in[i].message
						);
						in.erase(std::next(in.begin(), i));
						inbox[user_id[session[clientfd]]] = in;
						return;
					}
				}
			}
		},
	};

public:
	const int MAX_CONNECTION = 10;

	// initialize server
	Server(std::string ip, std::string port);

	// accept incoming connection, create thread for each connection
	void interactive();

	// handle operations
	void client_handler(int clientfd);

	int send(int clientfd, std::string message, bool newline = true);

	// wrapper of `send`, return what `send` return
	int send_banner(int clientfd);
	int send_prompt(int clientfd);
	int alert_if(int clientfd);

	std::pair<std::string, std::vector<std::string>> commandize(std::string s);
};

}

#endif  // _SERVER_HPP_
