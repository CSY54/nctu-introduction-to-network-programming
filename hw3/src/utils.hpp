#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <iostream>
#include <unistd.h>
#include <cstring>

#include "Argument.hpp"

#ifdef DEBUG
#define handle_error(...) \
	do { error(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#else
#define handle_error(...) error(__VA_ARGS__)
#endif

#define debug(...) _log("\033[1;33m[DEBUG]", __VA_ARGS__, "\033[0m")
#define error(...) _log("\033[0;31m[ERROR]", __VA_ARGS__, "\033[0m")
#define info(...) _log("\033[1;34m[INFO]", __VA_ARGS__, "\033[0m")
#define info_fd(...) _log_fd("\033[1;34m[INFO]", __VA_ARGS__, "\033[0m")

const std::string BASE64_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

template<typename T>
void _log(T &&_t) {
	std::cout << _t << std::endl;
}

template<typename T, typename ...S>
void _log(T &&_t, S &&..._s) {
	std::cout << _t << " ";
	_log(_s...);
}

template<typename T, typename S, typename ...U>
void _log_fd(T &&_t, S &&_s, U &&..._u) {
	std::cout << _t << " #" << _s << " ";
	_log(_u...);
}

const int MAX_BUFFER = 4096;

void close_connection(int);

int _send(int, std::string);

int sendline(int, std::string);

std::string recvline(int);

int send_banner(int);

int send_prompt(int);

std::pair<std::string, Argument> commandize(std::string);

bool guard(bool, int ,std::string);

bool is_int(std::string);

int to_int(std::string);

std::string _replace_all(std::string, std::string, std::string);

std::string filter(std::vector<std::string>, std::string);

std::string base64_encode(std::string);

std::string base64_decode(std::string);

#endif
