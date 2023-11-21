#include "server.hpp"
#include <poll.h>

Server::Server(int port) {
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
addrlen
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
    close(listenSocket);
	for (int i = 0; i < users.size(); ++i) {
		close(users[i].clientSocket);
	}
}

void Server::run() {
	int numberUsersAdd = 0;
	struct pollfd fds[20];
	for (int i = 0; i < 20; ++i) {
		fds[i].events = POLLOUT | POLLIN;
	}
	while (1) {
		if ((fds[numberUsersAdd].fd = accept(server_fd, (struct sockaddr*)&address, &addrlen) || numberUsersAdd++)
		    < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		int ret = poll(fds, numberUsersAdd, -1);
		if (ret > 0) {
			for (int i = 0; i < numberUsersAdd; ++i) {
				if (fds[i].revents & POLLIN) {
					recv(fds[i].fd, buffer, 1000, 0);
					std::cout << buffer << std::endl;
					for (int i = 0; i < 1000; ++i) {
						buffer[i] = 0;
					}
				}
				else if (fds[i].revents & POLLOUT && users[i].nickname != "" && users[i].username != "" && users[i].isInit == 0) {
					std::string welcomeMsg = ":irc 001 ONon :Welcome to the IRC Network, jtm\r\n";
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
