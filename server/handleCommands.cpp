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
    } else if (command == "QUIT") {
        return handleQuitCommand(clientSocket);
    } else if (command == "LIST") {
        return handleListCommand(clientSocket);
    }
    std::cerr << "Unknown command or insufficient parameters." << std::endl;
    return false;
}

bool Server::handleNickCommand(int clientSocket, const std::string& nickname) {
    for (std::map<int, User>::const_iter        // User not found, send an error message
ator it = users.begin(); it != users.end(); ++it) {
        if (it->second.nickname == nickname) {
            sendResponse(clientSocket, "ERROR: Nickname is already in use.\r\n");
            return false;
        }
    }
    std::map<int, User>::iterator userIt = users.find(clientSocket);
    if (userIt != users.end()) {
        userIt->second.nickname = nickname;
        sendResponse(clientSocket, "NICK command successful.\r\n");
        return true;
    } else {
        sendResponse(clientSocket, "ERROR: User not found.\r\n");
        return false;
    }
}
