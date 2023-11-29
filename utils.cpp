#include <vector>
#include <string>
#include <algorithm>
#include <set>

#include "server/server.hpp"
#include "user/user.hpp"

int findChannel(std::vector<Channel> channels, std::string name) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].getName() == name)
			return i;
	}
	return -1;
}

int getUserId(std::vector<User> users, std::string name) {
	std::cout << "/" << name << "/" << std::endl;
	for (size_t i = 0; i < users.size(); ++i) {
		if (users[i].nickname == name)
			return users[i].clientSocket;
	}
	return -1;
}

std::string onlyPrintable(const std::string &string) {
	std::string ret;
	int i = 0;
	while (isprint(string[i]) != 0) {
		ret.push_back(string[i]);
		i++;
	}
	return ret;
}