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

int getSocketId(std::vector<User> users, std::string name) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].nickname == name)
            return users[i].clientSocket;
    }
    return -1;
}

std::string getNickNameById(std::vector<User> users, int id) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].clientSocket == id)
            return users[i].nickname;
    }
    return "";
}

int getUserIdBySocketId(std::vector<User> users, int id) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].clientSocket == id)
            return i;
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

std::vector<std::string> splitString(std::string value, const char &sep) {
	std::vector<std::string> retValue;
	value = onlyPrintable(value);
	while (value.find(sep) != std::string::npos)
	{
		retValue.push_back(value.substr(0, value.find(sep)));
		value = value.substr(value.find(sep) + 1, value.size());
	}
	retValue.push_back(value);
	return retValue;
}

std::string multipleSpacesToOne(const std::string &value) {
    std::string newValue;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == ' ') {
            newValue.push_back(value[i]);
            i++;
            while (i < value.size() && value[i] == ' ')
                i++;
            i--;
            continue;
        }
        newValue.push_back(value[i]);
    }
    return newValue;
}

int isConflictNick(const std::vector<User> &users, const std::string &name) {
    for (size_t i = 0; i < users.size() - 1; ++i) {
        if (users[i].nickname == name)
            return -1;
    }
    return 0;
}

void sendInfoWhenJoin(const std::string &nickName, const std::vector<User> users, const Channel &channel) {
    if (channel.getTopic().empty()) {
        std::string topicmsg = ":irc 331 " + nickName + " " + channel.getName() + " :topic not set\r\n";
        send(getSocketId(users, nickName), topicmsg.c_str(), topicmsg.size(), 0);
    } else {
        std::string topicmsg = ":irc 332 " + nickName + " " + channel.getName() + " :" + channel.getTopic() + "\r\n";
        send(getSocketId(users, nickName), topicmsg.c_str(), topicmsg.size(), 0);
    }
    std::string names = ":irc 353 " + nickName + " = " + channel.getName() + " :";
    std::set<int>::iterator ite = channel.getUsers().end();
    for (std::set<int>::iterator it = channel.getUsers().begin(); it != ite; ++it) {
        if (it == channel.getUsers().begin()) {
            if (channel.isOperator(*it))
                names += '@';
            names += getNickNameById(users, *it);
        } else {
            names += ' ';
            if (channel.isOperator(*it))
                names += '@';
            names += getNickNameById(users, *it);
        }
    }
    std::cout << names << std::endl;
    names += "\r\n";
    send(getSocketId(users, nickName), names.c_str(), names.size(), 0);
    std::string endNames = ":irc 366 " + nickName + " " + channel.getName() + " :End of /NAMES list\r\n";
    send(getSocketId(users, nickName), endNames.c_str(), endNames.size(), 0);
}