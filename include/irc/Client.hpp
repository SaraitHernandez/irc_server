#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
// #include "MessageBuffer.hpp"

// Client class - represents a connected IRC client
// Manages client state, registration, and message buffer
class Client {
public:
    // Constructor: initialize with file descriptor
    // Client(int fd);
    
    // Destructor
    // ~Client();
    
    // File descriptor
    // int getFd() const;
    
    // Identity getters
    // const std::string& getNickname() const;
    // const std::string& getUsername() const;
    // const std::string& getHostname() const;
    // const std::string& getRealname() const;
    
    // Identity setters
    // void setNickname(const std::string& nickname);
    // void setUsername(const std::string& username, const std::string& realname);
    // void setHostname(const std::string& hostname);
    
    // Registration state
    // bool isRegistered() const;
    // bool hasPassword() const;
    // bool hasNickname() const;
    // bool hasUsername() const;
    // void setPassword(const std::string& password);
    // void registerClient();  // Mark as fully registered
    
    // Channel membership
    // bool isInChannel(const std::string& channelName) const;
    // bool isOperator(const std::string& channelName) const;
    // void joinChannel(const std::string& channelName);
    // void leaveChannel(const std::string& channelName);
    // std::vector<std::string> getChannels() const;
    
    // Message buffer management (for receiving data)
    // void appendToBuffer(const std::string& data);
    // std::string& getBuffer();
    // void clearBuffer();
    // MessageBuffer& getMessageBuffer();
    
    // Client prefix format: "nick!user@host"
    // std::string getPrefix() const;
    
private:
    // int fd_;
    // std::string nickname_;
    // std::string username_;
    // std::string hostname_;
    // std::string realname_;
    // std::string password_;
    
    // Registration flags
    // bool registered_;
    // bool passwordSet_;
    // bool nicknameSet_;
    // bool usernameSet_;
    
    // Channel membership
    // std::vector<std::string> channels_;
    
    // Message buffer for incomplete messages
    // MessageBuffer buffer_;
};

#endif // CLIENT_HPP

