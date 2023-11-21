#include "server.hpp"

Server::Server(int port) {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
    }

   int flags = fcntl(listenSocket, F_GETFL, 0);
    if (flags == -1) flags = 0;
    fcntl(listenSocket, F_SETFL, flags | O_NONBLOCK);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind to port " << port << std::endl;
    }
    if (listen(listenSocket, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket." << std::endl;
    }
    std::cout << "Server started on port " << port << std::endl;
}

Server::~Server() {
    close(listenSocket);
    for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it) {
        close(it->first);
    }
}



void Server::run() {
    //La loop principale avec le poll()
}

bool Server::acceptNewConnection() {
    //Accepte un user et la rajoute a la userMap
    return true;
}
