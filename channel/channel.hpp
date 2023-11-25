#include <string>
#include <set>

class Channel {
public:
    Channel(const std::string& name) : channelName(name) {}

    void setInviteOnly(bool value) {
        isInviteOnly = value;
    }

    bool getInviteOnly() const {
        return isInviteOnly;
    }

    void addInvitedUser(int userSocket) {
        invited.insert(userSocket);
    }

    bool isUserInvited(int userSocket) const {
        return invited.find(userSocket) != invited.end();
    }
    
    void addOperator(int userSocket) {
        operators.insert(userSocket);
    }

    void removeOperator(int userSocket) {
        operators.erase(userSocket);
    }

    bool isOperator(int userSocket) const {
        return operators.find(userSocket) != operators.end();
    }

    void setTopic(const std::string& topic) {
        channelTopic = topic;
    }

    bool addUser(int userSocket) {
        return users.insert(userSocket).second;
    }

    bool removeUser(int userSocket) {
        return users.erase(userSocket) > 0;
    }

    bool isUserInChannel(int userSocket) const {
        return users.find(userSocket) != users.end();
    }

    const std::string& getName() const { return channelName; }
    const std::string& getTopic() const { return channelTopic; }
    const std::set<int>& getUsers() const { return users; }

private:
    std::string channelName;
    std::string channelTopic;
    std::set<int> users;
    std::set<int> operators;
    std::set<int> invited;

    std::string channelKey;
    bool        isInviteOnly;
    bool        isTopicSecured;
    int         userNumberLimit;
};
