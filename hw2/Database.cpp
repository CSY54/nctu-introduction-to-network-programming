#include "Database.hpp"

Database::Database() {
	_board_id = 1;
	_post_id = 1;
}

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

void Database::create_board(BoardName name, Username moderator) {
	boards[name] = Board{
		_board_id,
		name,
		moderator
	};

	_board_id++;
}

std::vector<Board> Database::get_boards() {
	std::vector<Board> res;
	transform(
		boards.begin(),
		boards.end(),
		back_inserter(res),
		[](std::pair<BoardName, Board> board) {
			return board.second;
		}
	);

	sort(res.begin(), res.end(), [](Board lhs, Board rhs) {
		return lhs.id < rhs.id;
	});

	return res;
}

Board Database::get_board_by_name(BoardName name) {
	return boards.count(name) ? boards[name] : Board{};
}

void Database::create_post(PostTitle title, PostContent content, Username author, BoardName board_name) {
	std::time_t	t = std::time(nullptr);
	char mbstr[64];
	std::strftime(mbstr, sizeof(mbstr), "%m/%d", std::localtime(&t));

	posts[_post_id] = Post{
		_post_id,
		title,
		content,
		author,
		board_name,
		std::string(mbstr),
		std::vector<Comment>()
	};

	_post_id++;
}

std::vector<Post> Database::get_posts() {
	std::vector<Post> res;
	std::transform(
		posts.begin(),
		posts.end(),
		std::back_inserter(res),
		[](std::pair<PostId, Post> post) {
			return post.second;
		}
	);

	sort(res.begin(), res.end(), [](Post lhs, Post rhs) {
		return lhs.id < rhs.id;
	});

	return res;
}

Post Database::get_post_by_id(PostId id) {
	return posts.count(id) ? posts[id] : Post{};
}

std::vector<Post> Database::get_posts_by_board_name(BoardName board_name) {
	std::vector<Post> tmp;
	std::vector<Post> res;

	std::transform(
		posts.begin(),
		posts.end(),
		std::back_inserter(tmp),
		[](std::pair<PostId, Post> post) {
			return post.second;
		}
	);

	std::copy_if(
		tmp.begin(),
		tmp.end(),
		std::back_inserter(res),
		[&](Post post) {
			return post.board_name == board_name;
		}
	);

	sort(res.begin(), res.end(), [](Post lhs, Post rhs) {
		return lhs.id < rhs.id;
	});

	return res;
}

void Database::update_post_title_by_id(PostId id, PostTitle title) {
	posts[id].title = title;
}

void Database::update_post_content_by_id(PostId id, PostContent content) {
	posts[id].content = content;
}

void Database::delete_post_by_id(PostId id) {
	posts.erase(id);
}

void Database::create_comment(PostId id, Username username, CommentContent content) {
	posts[id].comments.emplace_back(Comment{username, content});
}

std::vector<User> Database::get_sessions() {
	std::vector<User> res;
	std::transform(
		sessions.begin(),
		sessions.end(),
		std::back_inserter(res),
		[](std::pair<SessionId, User> user) {
			return user.second;
		}
	);

	return res;
}

void Database::set_session(SessionId id, User user) {
	sessions[id] = user;
}

User Database::get_session(SessionId id) {
	return sessions.count(id) ? sessions[id] : User{};
}

void Database::clear_session(SessionId id) {
	sessions.erase(id);
}

bool Database::already_login(User user) {
	for (auto &[_, u] : sessions) {
		if (u == user) {
			return true;
		}
	}
	return false;
}
