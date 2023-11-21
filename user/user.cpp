#include "user.hpp"
#include <iostream>
#include <string>

User::User() {
    this->isInit = 0;
    this->nickname = "";
    this->username = "";
}

User::~User() {}

void User::setNickName(std::string value) {
    if (value.find("NICK") != std::string::npos) {
        nickname = value.substr( 5, value.size());
    }
}

void User::setUserName(std::string value) {
    if (value.find("USER") != std::string::npos) {
        username = value.substr(value.find(":") + 1, value.size());
    }
}
