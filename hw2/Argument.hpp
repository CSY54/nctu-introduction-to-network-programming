#ifndef __ARGUMENT_HPP__
#define __ARGUMENT_HPP__

#include <ostream>
#include <vector>
#include <map>

using args_t = std::vector<std::string>;
using kwargs_t = std::map<std::string, std::string>;

class Argument {
public:
	int argc;
	args_t args;
	kwargs_t kwargs;

	friend std::ostream& operator<<(std::ostream &os, const Argument argument) {
		os << std::endl;
		os << "argc: " << argument.argc << std::endl;
		os << "args:";
		if (argument.args.size() == 0) {
			os << " (null)";
		} else {
			for (auto &i : argument.args) {
				os << " " << i;
			}
		}
		os << std::endl;
		os << "kwargs:";
		if (argument.kwargs.size() == 0) {
			os << " (null)";
		} else {
			for (auto &[i, j] : argument.kwargs) {
				os << " {" << i << ": " << j << "}";
			}
		}
		return os;
	}
};

#endif
