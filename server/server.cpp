#include "server.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

Server::Server() {
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
	address.sin_port = htons(PORT);

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

void Server::run() {
    char buffer[1000] = {0};
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
                        processIncomingData(buffer, &fds, i);
                        std::cout << test << std::endl;
                        for (int i = 0; i < 1000; ++i) {
                            buffer[i] = 0;
                        }
                    }
                    else if (fds[i].revents & POLLOUT && users[i].nickname != "" && users[i].username != "" && users[i].isInit == 0) {
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
        std::cout << tmp << std::endl;
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
	users[i].setUserName(buffer);
	users[i].setNickName(buffer);
	if (buffer.find("QUIT") == 0)
		closeConnection(fds, i);
	std::vector<std::string> params;
//	std::cout << buffer.substr(0, buffer.find(' ')) << " " << (buffer.substr(0, buffer.find(' ')) == "QUIT") << std::endl;
//	handleCommand(users[i].clientSocket, buffer.substr(0, buffer.find(' ')), params);

    return true;
}