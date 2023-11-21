#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <set>
#include <string>


class User {
public:
	User();
	std::string nickname;
	std::string username;
	std::set<std::string> channels;
	int clientSocket;
	int isInit;
};

#endif //IRC_USER_HPP
