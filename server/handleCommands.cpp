#include "server.hpp"

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
            return handleJoinCommand(clientSocket, params[0]);
        }
    } else if (command == "PART") {
        if (params.size() >= 1) {
            return handlePartCommand(clientSocket, params[0]);
        }
    } else if (command == "PRIVMSG") {
        if (params.size() >= 2) {
            return handlePrivMsgCommand(clientSocket, params[0], params[1]);
        }
    } else if (command == "PING") {
        if (params.size() >= 1) {
            return handlePingCommand(clientSocket, params[0]);
        }
    } else if (command == "PONG") {
        if (params.size() >= 1) {
            return handlePongCommand(clientSocket, params[0]);
        }
    } else if (command == "LIST") {
        return handleListCommand(clientSocket);
    }
    std::cerr << "Unknown command or insufficient parameters." << std::endl;
    return false;
}

bool Server::handleNickCommand(int clientSocket, const std::string& nickname) {
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->nickname == nickname) {
            sendResponse(clientSocket, "ERROR: Nickname is already in use.\r\n");
            return false;
        }
    }
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->clientSocket == clientSocket) {
            it->nickname = nickname;
            sendResponse(clientSocket, "NICK command successful.\r\n");
            return true;
        }
    }
    sendResponse(clientSocket, "ERROR: User not found.\r\n");
    return false;
}


bool Server::handleUserCommand(int clientSocket, const std::string& username, const std::string& realname) {
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->clientSocket == clientSocket) {
            it->username = username;
            it->realname = realname;
            sendResponse(clientSocket, "Welcome " + username + "! Your name is set.\r\n");
            return true;
        }
    }
    sendResponse(clientSocket, "ERROR: User not found.\r\n");
    return false;
}


bool Server::handlePartCommand(int clientSocket, const std::string& channelName) {
    for (std::vector<User>::iterator userIt = users.begin(); userIt != users.end(); ++userIt) {
        if (userIt->clientSocket == clientSocket) {
            if (userIt->currentChannel != channelName) {
                sendResponse(clientSocket, "ERROR: Not in the specified channel.\r\n");
                return false;
            }
            std::map<std::string, Channel>::iterator channelIt = channels.find(channelName);
            if (channelIt == channels.end()) {
                sendResponse(clientSocket, "ERROR: Channel not found.\r\n");
                return false;
            }
            channelIt->second.removeUser(clientSocket);
            userIt->currentChannel.clear();
            sendResponse(clientSocket, "Left channel: " + channelName + "\r\n");
            return true;
        }
    }
    sendResponse(clientSocket, "ERROR: User not found.\r\n");
    return false;
}



bool Server::handlePrivMsgCommand(int clientSocket, const std::string& recipient, const std::string& message) {
    std::map<std::string, Channel>::iterator channelIt = channels.find(recipient);
    if (channelIt != channels.end()) {
        const std::set<int>& usersInChannel = channelIt->second.getUsers();
        for (int userSocket : usersInChannel) {
            if (userSocket != clientSocket) {
                sendResponse(userSocket, message);
            }
        }
        return true;
    }
    for (std::vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->nickname == recipient) {
            sendResponse(it->clientSocket, message);
            return true;
        }
    }
    sendResponse(clientSocket, "ERROR: Recipient not found.\r\n");
    return false;
}



bool Server::handlePingCommand(int clientSocket, const std::string& server) {
    std::string pongResponse = "PONG :" + server + "\r\n";
    sendResponse(clientSocket, pongResponse);
    return true;
}

bool Server::handlePongCommand(int clientSocket, const std::string& server) {
    return true;
}

bool Server::handleListCommand(int clientSocket) {
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        std::string channelInfo = "Channel: " + it->first + "\n Topic: " + it->second.getTopic() + "\r\n";
        sendResponse(clientSocket, channelInfo);
    }
    sendResponse(clientSocket, "End of channel list.\r\n");
    return true;
}