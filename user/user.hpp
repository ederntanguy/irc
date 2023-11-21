#include <unordered_set>
#include <string>

class User {
public:
    std::string nickname;
    std::string username;
    std::unordered_set<std::string> channels;
	int	clientSocket;

};
