#include <map>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "../user/user.hpp"
#include "../channel/channel.hpp"

#define PORT 6697
#define NUMBER_CLIENT_MAX 20

class Server {
public:
    Server();
    ~Server();

    void run();

private:
    int listenSocket;
    std::vector<User> users;
    std::map<std::string, Channel> channels;
    struct sockaddr_in address;
    socklen_t addrlen;
    int numberUsersAdd;

    bool acceptNewConnection(std::vector<struct pollfd> *fds);
    bool processIncomingData(const std::string& buffer, std::vector<struct pollfd> *fds, int i);
    void closeConnection(std::vector<struct pollfd> *fds, int i);

    bool sendResponse(int clientSocket, const std::string& message);
    bool handleCommand(int clientSocket, const std::string& command, const std::vector<std::string>& params);

    bool handleNickCommand(int clientSocket, const std::string& nickname);
    bool handleUserCommand(int clientSocket, const std::string& username, const std::string& realname);
    bool handleJoinCommand(int clientSocket, const std::string& channelName);
    bool handlePartCommand(int clientSocket, const std::string& channelName);
	bool handlePrivMsgCommand(int clientSocket, const std::string& recipient, const std::string& message);
	bool handlePingCommand(int clientSocket, const std::string& server);
	bool handlePongCommand();
	bool handleListCommand(int clientSocket);

    //void log(const std::string& message);
};
