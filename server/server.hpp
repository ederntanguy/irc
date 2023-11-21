#include <unordered_map>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "../user/user.h"
#include "../channel/channel.h"

class Server {
public:
    Server(int port);
    ~Server();

    void run();

private:
    int listenSocket;
    std::unordered_map<int, User> users;
    std::unordered_map<std::string, Channel> channels;

    bool acceptNewConnection();
    bool processIncomingData(int clientSocket);
    void closeConnection(int clientSocket);

    bool handleCommand(int clientSocket, const std::string& command, const std::vector<std::string>& params);

    bool handleNickCommand(int clientSocket, const std::string& nickname);
    bool handleUserCommand(int clientSocket, const std::string& username, const std::string& realname);
    bool handleJoinCommand(int clientSocket, const std::string& channelName);
    bool handlePartCommand(int clientSocket, const std::string& channelName);
	bool handlePrivMsgCommand(int clientSocket, const std::string& recipient, const std::string& message);
	bool handlePingCommand(int clientSocket, const std::string& server);
	bool handlePongCommand(int clientSocket, const std::string& server);
	bool handleQuitCommand(int clientSocket);

    //void log(const std::string& message);
};
