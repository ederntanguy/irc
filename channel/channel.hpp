#include <string>
#include <unordered_set>

class Channel {
public:
    Channel(const std::string& name) : channelName(name) {}

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

private:
    std::string channelName;
    std::string channelTopic;
    std::unordered_set<int> users;
};
