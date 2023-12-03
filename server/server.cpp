#include "server.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

Server::Server(long int port, const std::string &password) : address(), password(password) {
	int opt = 1;
    numberUsersAdd = 0;
	addrlen = sizeof(address);

	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(listenSocket, SOL_SOCKET,
	               SO_REUSEADDR | SO_REUSEPORT, &opt,
	               sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(listenSocket, (struct sockaddr*)&address,
	         sizeof(address))
	    < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(listenSocket, 20) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
    fcntl(listenSocket, F_SETFL, O_NONBLOCK);
}

Server::~Server() {
}

std::vector<std::string> multipleLine(std::string buffer) {
	size_t i = 0;
	size_t j;
	std::vector<std::string> ret;
	while (i < buffer.size()) {
		j = buffer.find('\n', i) + 1;
		ret.push_back(buffer.substr(i, j - i));
		i = j;
	}
	return ret;
}

void Server::run() {
    char buffer[10000] = {0};
    int ret;
    std::vector<struct pollfd> fds;

    while (1) {
        acceptNewConnection(&fds);
        if (numberUsersAdd > 0) {
            ret = poll(fds.data(), numberUsersAdd, -1);
            if (ret > 0) {
                for (int i = 0; i < numberUsersAdd; ++i) {
                    if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                        close(fds[i].fd);
                        fds[i].fd = 0;
                    }
                    else if (fds[i].revents & POLLIN) {
                        recv(fds[i].fd, buffer, 1000, 0);
                        std::string test(buffer);
						std::vector<std::string> allLine = multipleLine(test);
	                    for (size_t j = 0; j < allLine.size(); ++j) {
		                    if (!processIncomingData(allLine[j], &fds, i))
                                return;
                            if (users[i].nickname != "" && users[i].username != "" && users[i].isConnected == -1) {
                                sendResponse(users[i].clientSocket, ":irc 464 " + users[i].nickname + " :Password incorrect");
                                closeConnection(&fds, i);

                            }
                            std::cout << "/" << allLine[j] << "/" << std::endl;
	                    }
                        for (int i = 0; i < 1000; ++i) {
                            buffer[i] = 0;
                        }
                    }
                    else if (users[i].isInit == 0 && users[i].nickname != "" && users[i].username != "" && users[i].isConnected == 1) {
						std::string welcomeMsg = ":irc 001 " + users[i].nickname + " :Welcome to the IRC Network, " + users[i].nickname + "\r\n";
                        send(fds[i].fd, welcomeMsg.c_str(), welcomeMsg.size(), MSG_CONFIRM);
                        users[i].isInit = 1;
                    }
                }
            }
        }
	}
}

bool Server::acceptNewConnection(std::vector<struct pollfd> *fds) {
    int tmp = accept(listenSocket, (struct sockaddr*)&address, &addrlen);
    if (tmp > 0) {
        User newOne;
        newOne.clientSocket = tmp;
        users.push_back(newOne);
        numberUsersAdd++;
        struct pollfd tmp2;
        tmp2.fd = tmp;
        tmp2.events = POLLIN | POLLOUT;
        fds->push_back(tmp2);
    }

    return true;
}

void Server::closeConnection(std::vector<struct pollfd> *fds, int i) {
    std::vector<struct pollfd>::iterator itF = fds->begin();
    std::vector<User>::iterator itU = users.begin();
    std::advance(itF, i);
    std::advance(itU, i);
    close((*fds)[i].fd);
    users.erase(itU);
    fds->erase(itF);
    numberUsersAdd--;
}

bool Server::processIncomingData(const std::string& buffer, std::vector<struct pollfd> *fds, int i) {
    if (users[i].isConnected == 0 && !buffer.find("PASS")) {
        if (checkPassword(onlyPrintable(buffer.substr(buffer.find(' ') + 1, buffer.size())))) {
            users[i].isConnected = 1;
            return true;
        } else {
            std::cout << "ici :" << buffer << " | " <<  buffer.substr(buffer.find(' ') + 1, buffer.size()) << std::endl;
            users[i].isConnected = -1;
            return true;
        }
    }
	else if (users[i].setUserName(buffer))
		return true;
	else if (users[i].setNickName(buffer))
		return true;
	if (buffer.find("QUIT") == 0) {
		closeConnection(fds, i);
		return true;
	}
	else if (onlyPrintable(buffer) == "CAP LS 302")
		return true;
	std::vector<std::string> params;
	params.push_back(users[i].nickname);
	params.push_back(multipleSpacesToOne(onlyPrintable(buffer)));
	handleCommand(users[i].clientSocket, buffer.substr(0, buffer.find(' ')), params);
	return true;
}

bool Server::checkPassword(std::string value) {
    return value == this->password;
}
