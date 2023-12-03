#include "user.hpp"
#include <iostream>
#include <string>

User::User() {
    this->isInit = 0;
    this->nickname = "";
    this->username = "";
    this->isConnected = 0;
}

User::~User() {}

bool User::setNickName(std::string value) {
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
