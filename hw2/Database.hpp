#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__

#include <vector>
#include <map>
#include <algorithm>
#include <locale>

#include "Models.hpp"
#include "utils.hpp"

class Database {
private:
	std::map<Username, User> users;
	std::map<BoardName, Board> boards;
	std::map<PostId, Post> posts;
	std::map<SessionId, User> sessions;

	BoardId _board_id;
	PostId _post_id;

public:
	Database();

	// users
	void create_user(Username, Password);
	std::vector<User> get_users();
	User get_user_by_username(Username username);

	// boards
	void create_board(BoardName, Username);
	std::vector<Board> get_boards();
	Board get_board_by_name(BoardName);

	// posts
	void create_post(PostTitle, PostContent, Username, BoardName);
	std::vector<Post> get_posts();
	Post get_post_by_id(PostId);
	std::vector<Post> get_posts_by_board_name(BoardName);
	void update_post_title_by_id(PostId, PostTitle);
	void update_post_content_by_id(PostId, PostContent);
	void delete_post_by_id(PostId);
	void create_comment(PostId, Username, CommentContent);

	// sessions
	std::vector<User> get_sessions();
	void set_session(SessionId, User);
	User get_session(SessionId);
	void clear_session(SessionId);
	bool already_login(User);
};

#endif
