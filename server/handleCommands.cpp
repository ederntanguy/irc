#include "server.hpp"

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
            return handlePingCommand(clientSocket, params[0]);
        }
    } else if (command == "PONG") {
        if (params.size() >= 1) {
            return handlePongCommand();
        }
    } else if (command == "MODE") {
		std::string tmp = params[1].substr(params[1].find(' ') + 1, params[1].size());

	    return handleModeCommand(clientSocket, tmp);
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
//		std::map<std::string, Channel>::iterator channelIt = channels.find(channelNames[i]);
//		if (channelIt == channels.end()) {
//			sendResponse(clientSocket, ":irc ERROR Channel not found.\r\n");
//			return false;
//		}
//		if (!channelIt->second.isUserInChannel(clientSocket)) {
//			sendResponse(clientSocket, ":irc ERROR Not in the specified channel.\r\n");
//			return false;
//		}
//		channelIt->second.removeUser(clientSocket);
		sendResponse(clientSocket, ":" + params[0] + " PART :" + channelNames[i]);
	}
	return true;
}


bool Server::handlePrivMsgCommand(int clientSocket, const std::string &owner, const std::string& recipient, const std::string& message) {
	if (recipient[0] == '#') {
		for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
			if (it->getName() == recipient) {
				const std::set<int>& usersInChannel = it->getUsers();
				for (std::set<int>::const_iterator userIt = usersInChannel.begin(); userIt != usersInChannel.end(); ++userIt) {
					int userSocket = *userIt;
					if (userSocket != clientSocket) {
						sendResponse(userSocket, ":" + owner + " " + message);
					}
				}
				return true;
			}
		}
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
	if (server == "irc")
		pongResponse = "PONG :irc";
	else
		pongResponse = "PONG " + server;
	sendResponse(clientSocket, pongResponse);
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

std::string onlyAlphaNum(const std::string &string) {
	std::string ret;
	int i = 0;
	while (isprint(string[i]) != 0) {
		ret.push_back(string[i]);
		i++;
	}
	return ret;
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
		std::string tmpChalName = onlyAlphaNum(channelNames[i]);
        std::vector<Channel>::iterator channelIt = channels.begin();
        for (; channelIt != channels.end(); ++channelIt) {
            if (channelIt->getName() == tmpChalName) {
                if (channelIt->getInviteOnly() && !channelIt->isUserInvited(clientSocket)) {
                    sendResponse(clientSocket, ":irc ERROR " + tmpChalName + " is invite-only.");
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
	        sendResponse(clientSocket, ":" + params[0] + " JOIN :" +tmpChalName);
            return false;
        }
    }
    return true;
}

bool Server::handleModeCommand(int clientSocket, std::string& params) {
	const std::string channelName = params.substr(0, params.find(' '));
	params = params.substr(params.find(' ') + 1, params.size());
	const std::string modeParams = params.substr(0, params.find(' '));
	params = params.substr(params.find(' ') + 1, params.size());
	const std::string newModeParam = params.substr(0, params.find(' '));
	std::vector<Channel>::iterator channelIt = channels.begin();
    for (; channelIt != channels.end(); ++channelIt) {

        if (channelIt->getName() == channelName) {
            Channel& channel = *channelIt;
            bool settingMode = true;
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
		                channel.setInviteOnly(settingMode);
                        break;
                    case 't':
                        channel.setTopicSecured(settingMode);
                        break;
                    case 'k':
                        if (settingMode) {
                            channel.setChannelKey(newModeParam);
                        } else {
                            channel.removeChannelKey();
                        }
                        break;
                    case 'o':
                        {
                            int operatorId = atoi(newModeParam.c_str());
                            if (settingMode) {
                                channel.addOperator(operatorId);
                            } else {
                                channel.removeOperator(operatorId);
                            }
                        }
                        break;
                    case 'l':
                        {
                            int userLimit = atoi(newModeParam.c_str());
                            if (settingMode) {
                                channel.setUserLimit(userLimit);
                            } else {
                                channel.removeUserLimit();
                            }
                        }
                        break;
                    default:
                        sendResponse(clientSocket, ":irc ERROR Unknown mode.");
                        return false;
                }
			}
            sendResponse(clientSocket, "Mode set successfully.");
            return true;
        }
    }
    sendResponse(clientSocket, ":irc ERROR Channel does not exist.");
    return false;
}


