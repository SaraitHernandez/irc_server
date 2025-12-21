#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <ctime>
// #include "Client.hpp"

// Forward declaration
class Client;

// Channel class - represents an IRC channel
// Manages channel members, operators, topic, and modes
class Channel {
public:
    // Constructor: create channel with name
    // Channel(const std::string& name);
    
    // Destructor
    // ~Channel();
    
    // Identity
    // const std::string& getName() const;
    
    // Membership management
    // bool hasClient(Client* client) const;
    // void addClient(Client* client);
    // void removeClient(Client* client);
    // std::vector<Client*> getClients() const;
    // size_t getClientCount() const;
    
    // Operator management
    // bool isOperator(Client* client) const;
    // void addOperator(Client* client);
    // void removeOperator(Client* client);
    // std::vector<Client*> getOperators() const;
    
    // Topic management
    // const std::string& getTopic() const;
    // void setTopic(const std::string& topic, const std::string& setBy);
    // const std::string& getTopicSetter() const;
    // time_t getTopicTime() const;
    
    // Channel modes (i=invite-only, t=topic-protected, k=key-protected, o=operator, l=user-limit)
    // bool hasMode(char mode) const;
    // void setMode(char mode, bool value);
    // std::string getModeString() const;
    // std::string getChannelKey() const;
    // void setChannelKey(const std::string& key);
    // int getUserLimit() const;
    // void setUserLimit(int limit);
    
    // Broadcasting: send message to all members except exclude (if provided)
    // void broadcast(const std::string& message, Client* exclude = NULL);
    // void broadcastFrom(const std::string& message, Client* sender, Client* exclude = NULL);
    
    // Check if channel is empty (can be deleted)
    // bool isEmpty() const;
    
private:
    // std::string name_;
    // std::string topic_;
    // std::string topicSetter_;
    // time_t topicTime_;
    
    // Member storage
    // std::map<int, Client*> clients_;  // fd -> Client* for quick lookup
    // std::vector<Client*> operators_;   // List of operators
    
    // Channel modes
    // bool inviteOnly_;      // mode 'i'
    // bool topicProtected_;  // mode 't'
    // std::string channelKey_;  // mode 'k' (password)
    // int userLimit_;        // mode 'l'
};

#endif // CHANNEL_HPP

