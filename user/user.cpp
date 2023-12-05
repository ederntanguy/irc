#include "user.hpp"
#include <iostream>
#include <string>
#include <vector>

User::User() {
    this->isInit = 0;
    this->nickname = "";
    this->username = "";
    this->isConnected = 0;
}

User::~User() {
	std::cout << "proute" << std::endl;
}

bool User::setNickName(std::string value, std::vector<User> users) {
    if (value.find("NICK") != std::string::npos && nickname == "") {
        nickname = value.substr( 5, value.size());
        int i = 0;
        while(nickname[i]) {
            if (!isalnum(nickname[i])) {
                break;
            }
            i++;
        }
        nickname = nickname.substr(0, i);
	    isNickChecked = isConflictNick(users, nickname);
	    return true;
    }
	return false;
}

bool User::setUserName(std::string value) {
    if (value.find("USER") != std::string::npos && username == "") {
        username = value.substr(value.find(":") + 1, value.size() - 4);
        int i = 0;
        while(username[i]) {
            if (!isalnum(username[i])) {
                break;
            }
            i++;
        }
        username = username.substr(0, i);
	    return true;
	}
	return false;
}
