#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <vector>

#define PORT 6697
int main() {
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
	               SO_REUSEADDR | SO_REUSEPORT, &opt,
	               sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
	         sizeof(address))
	    < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	if ((new_socket
			     = accept(server_fd, (struct sockaddr*)&address,
			              &addrlen))
	    < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	// terminator at the end
	struct pollfd fds[100];
	fds[0].fd = new_socket;
	fds[0].events = POLLIN | POLLOUT;
	char buffer[1000] = {0};
	int b = 0;
    while (1) {
		int ret = poll(fds, 1, -1);
		if (ret > 0) {
			if (fds[0].revents & POLLIN) {
				recv(fds[0].fd, buffer, 1000, 0);
				std::cout << buffer << std::endl;
				for (int i = 0; i < 1000; ++i) {
					buffer[i] = 0;
				}
				b++;
			}
			else if (fds[0].revents & POLLOUT && b == 2) {
				std::string welcomeMsg = ":irc 001 ONon :Welcome to the IRC Network, jtm\r\n";
				send(fds[0].fd, welcomeMsg.c_str(), welcomeMsg.size(), MSG_CONFIRM);
				b++;
			}
		}
	}
	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	close(server_fd);
	return 0;
}
