#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <set>
#include <string>
#include <vector>
class User;
int isConflictNick(std::vector<User> users, std::string name);

class User {
public:
	User();
    ~User();
    bool setNickName(std::string value, std::vector<User> users);
    bool setUserName(std::string value);

	std::string nickname;
	std::string username;
	std::set<std::string> channels;
	int clientSocket;
	int isInit;
    int isConnected;
	int isNickChecked;
};

#endif //IRC_USER_HPP
