#include "Server.h"

Server::Server(int port) {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        exit(1);
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
        exit(1);
    }
    if (listen(listenSocket, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket." << std::endl;
        exit(1);
    }
    std::cout << "Server started on port " << port << std::endl;
}

Server::~Server() {
    close(listenSocket);
    for (auto& pair : users) {
        close(pair.first);
    }
}


void Server::run() {
    //La loop principale avec le poll()
}

bool Server::acceptNewConnection() {
    //Accepte un user et la rajoute a la userMap
}



