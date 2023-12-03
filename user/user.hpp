#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <set>
#include <string>


class User {
public:
	User();
    ~User();
    bool setNickName(std::string value);
    bool setUserName(std::string value);

	std::string nickname;
	std::string username;
	std::set<std::string> channels;
	int clientSocket;
	int isInit;
    int isConnected;
};

#endif //IRC_USER_HPP
