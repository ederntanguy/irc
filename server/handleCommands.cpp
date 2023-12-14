#include "server.hpp"
#include <algorithm>
#include <stdlib.h>

bool Server::sendResponse(int clientSocket, std::string msg) {
	msg.push_back('\r');
	msg.push_back('\n');
	send(clientSocket, msg.c_str(), msg.size(), 0);
	return true;
}

bool Server::handleCommand(int clientSocket, const std::string& command, const std::vector<std::string>& params) {
   if (command == "JOIN") {
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
            return handlePingCommand(clientSocket, params[1].substr(params[1].find(' ') + 1, params[1].size()));
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
    }
    std::cerr << "Unknown command or insufficient parameters." << std::endl;
    return false;
}

bool Server::handlePartCommand(int clientSocket, const std::vector<std::string>& params) {
	std::string infos = params[1].substr(params[1].find(' ') + 1, params[1].size());
	std::string tmpString = infos.substr(0, infos.find(' '));
	std::vector<std::string> channelNames = splitString(tmpString, ',');
	for (size_t i = 0; i < channelNames.size(); ++i) {
		if (channelNames[i][0] != '#' && channelNames[i][0] != '&') {
			sendResponse(clientSocket, ":irc ERROR " + channelNames[i] + " can't be a channel");
			continue;
		}
		int channelId = findChannel(channels, channelNames[i]);
		if (channelId == -1) {
			sendResponse(clientSocket, ":irc 403 " + params[0] + " " + channelNames[i] + " :No such channel");
			continue;
		}
		if (!channels[channelId].removeUser(clientSocket)) {
			sendResponse(clientSocket, ":irc 442 " + params[0] + " " + channelNames[i] + " :you are not in the channel");
			continue;
		}
		if (channels[channelId].isOperator(clientSocket))
			channels[channelId].removeOperator(clientSocket);
		sendResponse(clientSocket, ":" + params[0] + " PART :" + channelNames[i]);
        if (channels[channelId].getUsers().size() == 0) {
            channels.erase(channels.begin() + channelId);
        }
	}
	return true;
}


bool Server::handlePrivMsgCommand(int clientSocket, const std::string &owner, const std::string& recipient, const std::string& message) {
	if (recipient[0] == '#' || recipient[0] == '&') {
		int channelPos = findChannel(channels, recipient);
		if (channelPos == -1) {
            sendResponse(clientSocket, ":irc 401 " + owner + " " + recipient + " :No such channel");
			return false;
		}
		if (!channels[channelPos].isUserInChannel(clientSocket)) {
			sendResponse(clientSocket, ":irc 404 " + owner + " " + recipient + " :You are not in the channel");
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
        sendResponse(clientSocket, ":irc 401 " + owner + " " + recipient + " :No such user");

	}
    return false;
}

bool Server::handlePingCommand(int clientSocket, const std::string& server) {
	std::string pongResponse;
	sendResponse(clientSocket, "PONG " + server);
    return true;
}

bool Server::handlePongCommand() {
    return true;
}

bool Server::handleJoinCommand(int clientSocket, const std::vector<std::string> &params) {
	std::string infos = params[1].substr(params[1].find(' ') + 1, params[1].size());
	std::string tmpString = infos.substr(0, infos.find(' '));
	std::vector<std::string> channelsName = splitString(tmpString, ',');
	std::vector<std::string> channelsParam;
	if (infos.find(' ') != std::string::npos) {
		tmpString = infos.substr(infos.find(' ') + 1, infos.size());
		channelsParam = splitString(tmpString, ',');
	}
    for (size_t i = 0; i < channelsName.size(); ++i) {
		if (channelsName[i][0] != '#' && channelsName[i][0] != '&') {
			sendResponse(clientSocket, ":irc ERROR the channel name is not formated correctly ");
			return false;
		}
		std::string tmpChalName = channelsName[i];
        std::vector<Channel>::iterator channelIt = channels.begin();
        for (; channelIt != channels.end(); ++channelIt) {
            if (channelIt->getName() == tmpChalName) {
                if (channelIt->getInviteOnly() && !channelIt->isUserInvited(clientSocket)) {
                    sendResponse(clientSocket, ":irc 473 " + params[0] + " " + tmpChalName + " :(+i) is invite-only.");
	                break;
                }
				if (channelIt->isKeySet() && (channelsParam.size() <= i || !channelIt->isCorrectKey(channelsParam[i]))) {
                    if (channelsParam.size() <= i) {
                        sendResponse(clientSocket, ":irc 461 " + params[0] + " JOIN :Not enough parameters");
                        break;
                    }
					sendResponse(clientSocket, ":irc 475 " + params[0] + " " + tmpChalName + " :(+k) the password enter is not good");
					break;
				}
				if (channelIt->isUserLimitReached()) {
					sendResponse(clientSocket, ":irc 471 " + params[0] + " " + tmpChalName + " :(+l) the channel as reached is max user value");
					break;
				}
				if (!channelIt->addUser(clientSocket)) {
					sendResponse(clientSocket, ":irc ERROR can't add user to the channel");
					break;
				}
                sendResponse(clientSocket, ":" + params[0] + " JOIN :" + tmpChalName);
                sendInfoWhenJoin(params[0], users, *channelIt);
	            break;
            }
        }
        if (channelIt == channels.end()) {
			channels.push_back(Channel(tmpChalName));
			if (!channels[channels.size() - 1].addUser(clientSocket)) {
				sendResponse(clientSocket, ":irc ERROR can't add user to the channel");
				continue;
			}
	        channels[channels.size() - 1].addOperator(clientSocket);
	        sendResponse(clientSocket, ":" + params[0] + " JOIN :" +tmpChalName);
            sendInfoWhenJoin(params[0], users, channels[channels.size() - 1]);
        }
    }
    return true;
}

bool Server::handleModeCommand(int clientSocket,const std::string &nickName, std::string& params) {
	std::vector<std::string> allParams = splitString(params, ' ');
	if (allParams.size() < 2) {
        if (!allParams.empty()) {
            std::string msg = ":irc 324 " + nickName + " " + allParams[0] + " +";
            int channelId = findChannel(channels, allParams[0]);

            if (channels[channelId].getInviteOnly())
                msg += 'i';
            if (channels[channelId].getTopicSecured())
                msg += 't';
            if (channels[channelId].isKeySet())
                msg += 'k';
            if (channels[channelId].getUserLimit() > 0)
                msg += 'l';
            std::cout << msg << std::endl;
            sendResponse(clientSocket, msg);
        }
        else
            sendResponse(clientSocket, ":irc ERROR invalide number of parameters");
		return false;
	}
	const std::string channelName = allParams[0];
	const std::string modeParams = allParams[1];
	std::vector<std::string> infoParams;
	for (size_t i = 2; i < allParams.size(); ++i) {
		infoParams.push_back(allParams[i]);
	}
	int channelId = findChannel(channels, channelName);
	if (channelId == -1) {
		sendResponse(clientSocket, ":irc 403 " + nickName + " " + channelName + " :Channel does not exist.");
		return false;
	}
    if (!channels[channelId].isUserInChannel(clientSocket)) {
        sendResponse(clientSocket, ":irc 442 " + nickName + " " + channelName + " :you are not in the channel");
        return false;
    }
	if (!channels[channelId].isOperator(clientSocket)) {
		sendResponse(clientSocket, ":irc 482 " + nickName + " " + channelName + " :you are not a operator in the channel " + channelName);
		return false;
	}
	bool settingMode = true;
	std::set<int> usersChannel = channels[channelId].getUsers();
	size_t posInfoParams = 0;
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
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + (settingMode == true ? '+' : '-') + mode);
				}
				break;
			case 't':
				channels[channelId].setTopicSecured(settingMode);
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + (settingMode == true ? '+' : '-') + mode);
				}
				break;
			case 'k':
				if (settingMode) {
                    if (infoParams.size() <= posInfoParams) {
                        sendResponse(clientSocket, ":irc 461 " + nickName + " " + mode + " :Not enough parameters");
                        continue;
                    }
					channels[channelId].setChannelKey(infoParams[posInfoParams]);
					for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
						sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + (settingMode == true ? '+' : '-') + mode + " " + infoParams[posInfoParams]);
					}
					posInfoParams++;
				} else {
					channels[channelId].removeChannelKey();
					for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
						sendResponse((*it), ":" + nickName + " MODE " + channelName + " " + (settingMode == true ? '+' : '-') + mode);
					}
				}
				break;
			case 'o': {
                if (infoParams.size() <= posInfoParams) {
                    sendResponse(clientSocket, ":irc 461 " + nickName + " " + mode + " :Not enough parameters");
                    continue;
                }
				int operatorId = getSocketId(users, infoParams[posInfoParams]);
                if (operatorId == -1) {
                    sendResponse(clientSocket, ":irc 401 " + nickName + " " + infoParams[posInfoParams] + " :No such user");
                    continue;
                }
                if (!channels[channelId].isUserInChannel(operatorId)) {
                    sendResponse(clientSocket, ":irc 441 " + nickName + " " + infoParams[posInfoParams] + " :the user is not in the channel");
                    continue;
                }
				if (settingMode) {
					channels[channelId].addOperator(operatorId);
				} else {
					channels[channelId].removeOperator(operatorId);
				}
				for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
					sendResponse((*it),":" + nickName + " MODE " + channelName + " " + (settingMode == true ? '+' : '-') + mode + " " + infoParams[posInfoParams]);
				}
				posInfoParams++;
			}
				break;
			case 'l':
			{
				if (settingMode) {
                    if (infoParams.size() <= posInfoParams) {
                        sendResponse(clientSocket, ":irc 461 " + nickName + " " + mode + " :Not enough parameters");
                        continue;
                    }
                    int userLimit = atoi(infoParams[posInfoParams].c_str());
					channels[channelId].setUserLimit(userLimit);
					for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
						sendResponse((*it), ":" + nickName + " MODE " + channelName + " +" + mode + " " + infoParams[posInfoParams]);
					}
                    posInfoParams++;
				} else {
					channels[channelId].removeUserLimit();
					for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
						sendResponse((*it), ":" + nickName + " MODE " + channelName + " -" + mode);
					}
				}
			}
				break;
			default:
			{
				sendResponse(clientSocket, ":irc 472 " + nickName + " " + channelName + " :Unknown mode.");
				return false;
			}
		}
    }
	return true;
}

bool Server::handleKickCommand(int clientSocket, const std::vector<std::string> &params) {
	int i = params[1].find(' ', params[1].find(' ') + 1);
	std::string channel = params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1);
	std::string userKick = params[1].substr(i + 1, params[1].find(' ', i + 1) - i - 1);
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
        sendResponse(clientSocket, ":irc 403 " + params[0] + " " + channel + " :No such channel");
		return false;
	}
    if (!channels[channelPos].isUserInChannel(clientSocket)) {
        sendResponse(clientSocket, ":irc 442 " + params[0] + " " + channel + " :you are not in the channel");
        return false;
    }
	if (!channels[channelPos].isOperator(clientSocket)) {
        sendResponse(clientSocket, ":irc 482 " + params[0] + " " + channel + " :you are not a operator in the channel " + channel);
		return false;
	}
	int userId = getSocketId(users, userKick);
	if (userId == -1) {
        sendResponse(clientSocket, ":irc 401 " + params[0] + " " + userKick + " :No such channel");
		return false;
	}
	if (!channels[channelPos].isUserInChannel(userId)) {
        sendResponse(clientSocket, ":irc 441 " + params[0] + " " + userKick + " :the user is not in the channel");
		return false;
	}
	channels[channelPos].removeUser(userId);
	sendResponse(userId, ":" + params[0] + " " + params[1]);
	std::set<int> usersChannel = channels[channelPos].getUsers();
	for (std::set<int>::iterator it = usersChannel.begin(); it != usersChannel.end(); ++it) {
		sendResponse((*it), ":" + params[0] + " " + params[1]);
	}
	return true;
}

bool Server::handleInviteCommand(int clientSocket, const std::vector<std::string> &params) {
	int i = params[1].find(' ', params[1].find(' ') + 1);
	std::string userInvite = params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1);
	std::string channel = params[1].substr(i + 1, params[1].find(' ', i + 1) - i - 1);
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
        sendResponse(clientSocket, ":irc 403 " + params[0] + " " + channel + " :No such channel");
		return false;
	}
    if (!channels[channelPos].isUserInChannel(clientSocket)) {
        sendResponse(clientSocket, ":irc 442 " + params[0] + " " + channel + " :you are not in the channel");
        return false;
    }
    if (!channels[channelPos].isOperator(clientSocket) && channels[channelPos].getInviteOnly()) {
        sendResponse(clientSocket, ":irc 482 " + params[0] + " " + channel + " :you are not a operator in the channel");
		return false;
	}
    int userId = getSocketId(users, userInvite);
    if (userId == -1) {
        sendResponse(clientSocket, ":irc 401 " + params[0] + " " + userInvite + " :No such user in channel");
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
	std::string channel = params[1].substr(params[1].find(' ') + 1, i - params[1].find(' ') - 1);
	std::string topic;
	if (params[1].find(':') == std::string::npos)
		topic = "";
	else {
		topic = params[1].substr(params[1].find(':'), params[1].size());
	}
	int channelPos = findChannel(channels, channel);
	if (channelPos == -1) {
        sendResponse(clientSocket, ":irc 403 " + params[0] + " " + channel + " :No such channel");
		return false;
	}
    if (!channels[channelPos].isUserInChannel(clientSocket)) {
        sendResponse(clientSocket, ":irc 442 " + params[0] + " " + channel + " :you are not in the channel");
        return false;
    }
	if (channels[channelPos].getTopicSecured() && !channels[channelPos].isOperator(clientSocket)) {
        sendResponse(clientSocket, ":irc 482 " + params[0] + " " + channel + " :you are not a operator in the channel");
		return false;
	}
	if (topic.empty()) {
		if (channels[channelPos].getTopic().empty()) {
			sendResponse(clientSocket, ":irc 331 " + params[0] + " " + channel + " :topic not set");
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
