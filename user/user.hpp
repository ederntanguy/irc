#include <set>
#include <string>

class User {
public:
    std::string nickname;
    std::string username;
    std::set<std::string> channels;
    int clientSocket;
};
