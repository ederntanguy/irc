#include "server.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

Server::Server() {
	int opt = 1;
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
}

Server::~Server() {
}

void Server::run() {
	int numberUsersAdd = 0;
	struct pollfd fds[20];
    char buffer[1000] = {0};
	for (int i = 0; i < 20; ++i) {
		fds[i].events = POLLOUT | POLLIN;
        fds[i].fd = 0;
	}
    if ((fds[numberUsersAdd].fd = accept(listenSocket, (struct sockaddr*)&address, &addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    if (fds[numberUsersAdd].fd != 0) {
        User newOne;
        newOne.clientSocket = fds[numberUsersAdd].fd;
        users.push_back(newOne);
        numberUsersAdd++;
    }
    while (1) {
        int ret = poll(fds, numberUsersAdd, -1);
        if (ret > 0) {
            for (int i = 0; i < numberUsersAdd; ++i) {
                if (fds[i].revents & POLLIN) {
                    recv(fds[i].fd, buffer, 1000, 0);
                    std::cout << buffer << std::endl;
                    if (users[i].username == "")
                        users[i].setUserName(buffer);
                    if (users[i].nickname == "")
                        users[i].setNickName(buffer);
                    for (int i = 0; i < 1000; ++i) {
                        buffer[i] = 0;
                    }
                }
                else if (fds[i].revents & POLLOUT && users[i].nickname != "" && users[i].username != "" && users[i].isInit == 0) {
                    std::string welcomeMsg = ":irc 001 " + users[i].nickname + " :Welcome to the IRC Network, " + users[i].nickname + "\r\n";
                    send(fds[i].fd, welcomeMsg.c_str(), welcomeMsg.size(), MSG_CONFIRM);
                    users[i].isInit = 1;
                }
				else if (fds[i].revents & POLLREMOVE) {
					close(fds[i].fd);
					fds[i].fd = 0;
				}
            }
        }
	}
}

bool Server::acceptNewConnection() {
    //Accepte un user et la rajoute a la userMap
    return true;
}
