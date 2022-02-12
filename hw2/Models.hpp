#ifndef __MODELS_HPP__
#define __MODELS_HPP__

#include <ostream>
#include <vector>

using Username = std::string;
using Password = std::string;
using BoardId = int;
using BoardName = std::string;
using PostId = int;
using PostTitle = std::string;
using PostContent = std::string;
using Date = std::string;
using CommentContent = std::string;
using SessionId = int;

struct User {
	Username username;
	Password password;

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

struct Board {
	BoardId id;
	BoardName name;
	Username moderator;

	// only check primary key
	bool operator==(const Board &rhs) const {
		return name == rhs.name;
	}

	bool operator!=(const Board &rhs) const {
		return !(*this == rhs);
	}

	friend std::ostream& operator<<(std::ostream &os, const Board &board) {
		os << "[Board]" << std::endl
			<< "\tid: " << board.id << std::endl
			<< "\tname: " << board.name << std::endl
			<< "\tmoderator: " << board.moderator;
		return os;
	}
};

struct Comment {
	Username username;
	CommentContent content;

	friend std::ostream& operator<<(std::ostream &os, const Comment &comment) {
		os << comment.username << ": " << comment.content;
		return os;
	}
};

struct Post {
	PostId id;
	PostTitle title;
	PostContent content;
	Username author;
	BoardName board_name;
	Date date;
	std::vector<Comment> comments;

	// only check primary key
	bool operator==(const Post &rhs) const {
		return id == rhs.id;
	}

	bool operator!=(const Post &rhs) const {
		return !(*this == rhs);
	}

	friend std::ostream& operator<<(std::ostream &os, const Post &post) {
		os << "Author: " << post.author << std::endl
			<< "Title: " << post.title << std::endl
			<< "Date: " << post.date << std::endl
			<< "--" << std::endl
			<< post.content << std::endl
			<< "--" << std::endl;
		for (auto &comment : post.comments) {
			os << comment << std::endl;
		}
		return os;
	}
};

struct Session {
	SessionId id;
	User user;

	friend std::ostream& operator<<(std::ostream &os, const Session &session) {
		os << session.id << " " << session.user;
		return os;
	}
};

#endif
