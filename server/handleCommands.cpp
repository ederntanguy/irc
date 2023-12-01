#include "server.hpp"
#include <algorithm>

// utils function
int findChannel(std::vector<Channel> channels, std::string name);
int getUserId(std::vector<User> users, std::string name);
std::string onlyPrintable(const std::string &string);


bool Server::sendResponse(int clientSocket, std::string msg) {
	msg.push_back('\r');
	msg.push_back('\n');
	send(clientSocket, msg.c_str(), msg.size(), 0);
	return true;
}

bool Server::handleCommand(int clientSocket, const std::string& command, const std::vector<std::string>& params) {
    if (command == "NICK") {
        if (params.size() >= 1) {
            return handleNickCommand(clientSocket, params[0]);
        }
    } else if (command == "USER") {
        if (params.size() >= 4) {
            return handleUserCommand(clientSocket, params[0], params[3]);
        }
    } else if (command == "JOIN") {
        if (params.size() >= 1) {
            return handleJoinCommand(clientSocket, params);
        }
    } else if (command == "PART") {
        if (params.size() >= 1) {
            return handlePartCommand(clientSocket, params);
        }
    } else if (command == "PRIVMSG") {
        if (params.size() >= 2) {
            return handlePrivMsgCommand(clientSocket,
										params[0], params[1].substr(params[1].find(' ') + 1, params[1].find(' ', params[1].find(' ') + 1) - (params[1].find(' ') + 1)),
										params[1]);
        }
    } else if (command == "PING") {
        if (params.size() >= 1) {
            return handlePingCommand(clientSocket, onlyPrintable(params[1].substr(params[1].find(' ') + 1, params[1].size())));
        }
    } else if (command == "PONG") {
        if (params.size() >= 1) {
            return handlePongCommand();
        }
    } else if (command == "MODE") {
	    std::string tmp = params[1].substr(params[1].find(' ') + 1, params[1].size());
	    return handleModeCommand(clientSocket, params[0], tmp);
    } else if (command == "KICK") {
	    return handleKickCommand(clientSocket, params);
    } else if (command == "INVITE") {
	    return handleInviteCommand(clientSocket, params);
    } else if (command == "TOPIC") {
	    return handleTopicCommand(clientSocket, params);
    } else if (command == "LIST") {
	    return handleListCommand(clientSocket);
    }
    std::cerr << "Unknown command or insufficient parameters." << std::endl;
    return false;
}

bool Server::handleNickCommand(int clientSocket, const std::string& nickname) {
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->nickname == nickname) {
            sendResponse(clientSocket, ":irc ERROR Nickname is already in use.");
            return false;
        }
    }
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->clientSocket == clientSocket) {
            it->nickname = nickname;
            sendResponse(clientSocket, "NICK command successful.");
            return true;
        }
    }
    sendResponse(clientSocket, ":irc ERROR User not found.");
    return false;
}

bool Server::handleUserCommand(int clientSocket, const std::string& username, const std::string& nickname) {
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->clientSocket == clientSocket) {
            it->username = username;
            it->nickname = nickname;
            sendResponse(clientSocket, "Welcome " + username + "! Your name is set.");
            return true;
        }
    }
    sendResponse(clientSocket, ":irc ERROR User not found.");
    return false;
}

bool Server::handlePartCommand(int clientSocket, const std::vector<std::string>& params) {
	size_t tmp = params[1].find(' ') + 1;
	size_t res1, res2;
	std::vector<std::string> channelNames;
	while (params[1].size() > tmp) {
		res1 = params[1].find(',', tmp);
		res2 = params[1].find(' ', tmp);
		if (res1 < res2) {
			channelNames.push_back(params[1].substr(tmp, res1 - tmp));
			tmp = res1 + 1;
		} else if (res1 > res2) {
			channelNames.push_back(params[1].substr(tmp, res2 - tmp));
			tmp = res2 + 1;
		} else if (res1 >= params[1].size()) {
			channelNames.push_back(params[1].substr(tmp, res1 - tmp - 1));
			tmp = params[1].size();
		} else {
			std::cerr << "wow trop bizarre la" << std::endl;
			return false;
		}
	}
	for (size_t i = 0; i < channelNames.size(); ++i) {
		if (channelNames[i][0] != '#' && channelNames[i][0] != '&') {
			sendResponse(clientSocket, ":irc ERROR " + channelNames[i] + " can't be a channel");
			return false;
		}
		sendResponse(clientSocket, ":" + params[0] + " PART :" + channelNames[i]);
	}
	return true;
}


bool Server::handlePrivMsgCommand(int clientSocket, const std::string &owner, const std::string& recipient, const std::string& message) {
	if (recipient[0] == '#') {
		int channelPos = findChannel(channels, recipient);
		if (channelPos == -1) {
			sendResponse(clientSocket, ":irc ERROR The channel " + recipient + " does not exist");
			return false;
		}
		if (!channels[channelPos].isUserInChannel(clientSocket)) {
			sendResponse(clientSocket, ":irc ERROR you are not in the channel " + recipient);
			return false;
		}
		std::set<int> usersId = channels[channelPos].getUsers();
		for (std::set<int>::iterator it = usersId.begin(); it != usersId.end(); it++) {
			if (*it != clientSocket) {
				sendResponse(*it, ":" + owner + " " + message);
			}
		}
		return true;
	} else {
		for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
			if (it->nickname == recipient) {
				sendResponse(it->clientSocket, ":" + owner + " " + message);
				return true;
			}
		}
	}
    sendResponse(clientSocket, ":irc ERROR Recipient not found.");
    return false;
}

bool Server::handlePingCommand(int clientSocket, const std::string& server) {
	std::string pongResponse;
	(void)server;
	sendResponse(clientSocket, "PONG " + server);
    return true;
}

bool Server::handlePongCommand() {
    return true;
}

bool Server::handleListCommand(int clientSocket) {
    for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        std::string channelInfo = "Channel: " + it->getName() + "\n Topic: " + it->getTopic();
        sendResponse(clientSocket, channelInfo);
    }
    sendResponse(clientSocket, "End of channel list.");
    return true;
}

bool Server::handleJoinCommand(int clientSocket, const std::vector<std::string> &params) {
	size_t tmp = params[1].find(' ') + 1;
	size_t res1, res2;
	std::vector<std::string> channelNames;
	while (params[1].size() > tmp) {
		res1 = params[1].find(',', tmp);
		res2 = params[1].find(' ', tmp);
		if (res1 < res2) {
			channelNames.push_back(params[1].substr(tmp, res1 - tmp));
			tmp = res1 + 1;
		} else if (res1 > res2) {
			channelNames.push_back(params[1].substr(tmp, res2 - tmp));
			tmp = res2 + 1;
		} else if (res1 >= params[1].size()) {
			channelNames.push_back(params[1].substr(tmp, res1 - tmp - 1));
			tmp = params[1].size();
		} else {
			std::cerr << "wow trop bizarre la" << std::endl;
			return false;
		}
	}
    for (size_t i = 0; i < channelNames.size(); ++i) {
		if (channelNames[i][0] != '#' && channelNames[i][0] != '&') {
			sendResponse(clientSocket, ":irc ERROR the channel name is not formated correctly ");
			return false;
		}
		std::string tmpChalName = onlyPrintable(channelNames[i]);
        std::vector<Channel>::iterator channelIt = channels.begin();
        for (; channelIt != channels.end(); ++channelIt) {
            if (channelIt->getName() == tmpChalName) {
                if (channelIt->getInviteOnly() && !channelIt->isUserInvited(clientSocket)) {
                    sendResponse(clientSocket, ":irc ERROR " + tmpChalName + " is invite-only.");
                    return false;
                }
				if (channelIt->isUserLimitReached()) {
					sendResponse(clientSocket, ":irc ERROR the channel as reached is max user value");
					return false;
				}
				if (!channelIt->addUser(clientSocket)) {
					sendResponse(clientSocket, ":irc ERROR can't add user to the channel");
					return false;
				}
                sendResponse(clientSocket, ":" + params[0] + " JOIN :" + tmpChalName);
                break;
            }
        }
        if (channelIt == channels.end()) {
			channels.push_back(Channel(tmpChalName));
			if (!channels[channels.size() - 1].addUser(clientSocket)) {
				sendResponse(clientSocket, ":irc ERROR can't add user to the channel");
				return false;
			}
	        channels[channels.size() - 1].addOperator(clientSocket);
	        sendResponse(clientSocket, ":" + params[0] + " JOIN :" +tmpChalName);
            return false;
        }
    }
    return true;
}

bool Server::handleModeCommand(int clientSocket,const std::string &nickName, std::string& params) {
	params = onlyPrintable(params);
	const std::string channelName = params.substr(0, params.find(' '));
	params = params.substr(params.find(' ') + 1, params.size());
	const std::string modeParams = params.substr(0, params.find(' '));
	params = params.substr(params.find(' ') + 1, params.size());
	const std::string newModeParam = params.substr(0, params.find(' '));
	std::cout << "la : " << channelName << "  " << modeParams << "  " << newModeParam << std::endl;
	int channelId = findChannel(channels, channelName);
	if (channelId == -1) {
		sendResponse(clientSocket, ":irc ERROR Channel does not exist.");
		return false;
	}
	if (!channels[channelId].isOperator(clientSocket)) {
		sendResponse(clientSocket, ":irc ERROR you are not a operator in the channel " + channelName);
		return false;
	}
	bool settingMode = true;
	std::set<int> usersChannel = channels[channelId].getUsers();
	for (std::string::const_iterator it = modeParams.begin(); it != modeParams.end(); ++it) {
		char mode = *it;
		switch (mode) {
			case '+':
				settingMode = true;
				break;
			case '-':
				settingMode = false;
				break;
			case 'i':
				channels[channelId].setInviteOnly(settingMode);
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + modeParams);
				}
				break;
			case 't':
				channels[channelId].setTopicSecured(settingMode);
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + modeParams);
				}
				break;
			case 'k':
				if (settingMode) {
					channels[channelId].setChannelKey(newModeParam);
				} else {
					channels[channelId].removeChannelKey();
				}
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + modeParams + " " + newModeParam);
				}
				break;
			case 'o': {

				int operatorId = atoi(newModeParam.c_str());
				if (settingMode) {
					channels[channelId].addOperator(operatorId);
				} else {
					channels[channelId].removeOperator(operatorId);
				}
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it),":" + nickName + " MODE " + channelName + " " + modeParams + " " + newModeParam);
				}
			}
				break;
			case 'l':
			{
				int userLimit = atoi(newModeParam.c_str());
				if (settingMode) {
					channels[channelId].setUserLimit(userLimit);
				} else {
					channels[channelId].removeUserLimit();
				}
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + modeParams + " " + newModeParam);
				}
			}
				break;
			default:
			{
				sendResponse(clientSocket, ":irc ERROR Unknown mode.");
				return false;
			}
		}
    }
	return true;
}

bool Server::handleKickCommand(int clientSocket, const std::vector<std::string> &params) {
	int i = params[1].find(' ', params[1].find(' ') + 1);
	std::string channel = onlyPrintable(params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1));
	std::string userKick = onlyPrintable(params[1].substr(i + 1, params[1].find(' ', i + 1) - i - 1));
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
		sendResponse(clientSocket, ":irc ERROR The channel " + channel + " does not exist");
		return false;
	}
	if (!channels[channelPos].isOperator(clientSocket)) {
		sendResponse(clientSocket, ":irc ERROR you are not a operator in the channel " + channel);
		return false;
	}
	int userId = getUserId(users, userKick);
	if (userId == -1) {
		sendResponse(clientSocket, ":irc ERROR The user " + userKick + " does not exist");
		return false;
	}
	if (!channels[channelPos].isUserInChannel(userId)) {
		sendResponse(clientSocket, ":irc ERROR The user " + userKick + " does not exist in the channel " + channel);
		return false;
	}
	channels[channelPos].removeUser(userId);
	std::cout << "la/" << ":" + params[0] + " " + onlyPrintable(params[1]) << "/al" << std::endl;
	sendResponse(userId, ":" + params[0] + " " + onlyPrintable(params[1]));
	std::set<int> usersChannel = channels[channelPos].getUsers();
	for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
		sendResponse((*it), ":" + params[0] + " " + params[1]);
	}
	return true;
}

bool Server::handleInviteCommand(int clientSocket, const std::vector<std::string> &params) {
	int i = params[1].find(' ', params[1].find(' ') + 1);
	std::string userInvite = onlyPrintable(params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1));
	std::string channel = onlyPrintable(params[1].substr(i + 1, params[1].find(' ', i + 1) - i - 1));
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
		sendResponse(clientSocket, ":irc ERROR The channel " + channel + " does not exist");
		return false;
	}
	if (!channels[channelPos].isOperator(clientSocket) && channels[channelPos].getInviteOnly()) {
		sendResponse(clientSocket, ":irc ERROR you are not a operator in the channel " + channel);
		return false;
	}
	int userId = getUserId(users, userInvite);
	if (userId == -1) {
		sendResponse(clientSocket, ":irc ERROR The user " + userInvite + " does not exist");
		return false;
	}
	if (channels[channelPos].isUserInvited(userId)) {
		sendResponse(clientSocket, ":irc ERROR The user " + userInvite + " is already invited");
		return false;
	}
	channels[channelPos].addInvitedUser(userId);
	sendResponse(userId, ":" + params[0] + " " + params[1]);
	return true;
}

bool Server::handleTopicCommand(int clientSocket, const std::vector<std::string> &params) {
	int i = params[1].find(' ', params[1].find(' ') + 1);
	std::string channel = onlyPrintable(params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1));
	std::string topic;
	if (params[1].find(':') == std::string::npos)
		topic = "";
	else {
		topic = params[1].substr(params[1].find(':'), params[1].size());
	}
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
		sendResponse(clientSocket, ":irc ERROR The channel " + channel + " does not exist");
		return false;
	}
	if (channels[channelPos].getTopicSecured() && !channels[channelPos].isOperator(clientSocket)) {
		sendResponse(clientSocket, ":irc ERROR you are not a operator in the channel " + channel);
		return false;
	}
	if (topic.empty()) {
		if (channels[channelPos].getTopic().empty()) {
			sendResponse(clientSocket, ":irc 331 " + params[0] + " " + channel + " :topic does not exist");
			return false;
		} else {
			sendResponse(clientSocket, ":irc 332 " + params[0] + " " + channel + " :" + channels[channelPos].getTopic());
			return true;
		}
	}
	channels[channelPos].setTopic(topic);
	std::set<int> usersChannel = channels[channelPos].getUsers();
	for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end() ; ++it) {
		sendResponse((*it), ":" + params[0] + " " + params[1]);
	}
	return true;
}


