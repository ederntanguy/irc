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
#include <stdlib.h>

#define NUMBER_CLIENT_MAX 20
int findChannel(std::vector<Channel> channels, std::string name);
int getSocketId(std::vector<User> users, std::string name);
std::string onlyPrintable(const std::string &string);
std::vector<std::string> splitString(std::string &value, char sep);
int getUserIdBySocketId(std::vector<User> users, int id);

class Server {
public:
    Server(long int port, const std::string &password);
    ~Server();

    void run();

private:
    int listenSocket;
    std::vector<User> users;
    std::vector<Channel> channels;
    struct sockaddr_in address;
    socklen_t addrlen;
    int numberUsersAdd;
    std::string password;

    bool checkPassword(std::string value);

    bool acceptNewConnection(std::vector<struct pollfd> *fds);
    bool processIncomingData(const std::string& buffer, std::vector<struct pollfd> *fds, int i);
    void closeConnection(std::vector<struct pollfd> *fds, int i);

    bool sendResponse(int clientSocket, std::string message);
    bool handleCommand(int clientSocket, const std::string& command, const std::vector<std::string>& params);

    bool handleNickCommand(int clientSocket, const std::string& nickname);
    bool handleUserCommand(int clientSocket, const std::string& username, const std::string& realname);
    bool handleJoinCommand(int clientSocket, const std::vector<std::string> &params);
    bool handlePartCommand(int clientSocket, const std::vector<std::string>& params);
	bool handlePrivMsgCommand(int clientSocket, const std::string &owner, const std::string& recipient, const std::string& message);
	bool handlePingCommand(int clientSocket, const std::string& server);
	bool handlePongCommand();
	bool handleKickCommand(int clientSocket, const std::vector<std::string> &params);
	bool handleInviteCommand(int clientSocket, const std::vector<std::string> &params);
	bool handleTopicCommand(int clientSocket, const std::vector<std::string> &params);
	bool handleListCommand(int clientSocket);
	bool handleModeCommand(int clientSocket,const std::string &nickName, std::string& params);
    //void log(const std::string& message);
};
