#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>


// Client class - represents a connected IRC client
// Network Layer (Alex): fd, hostname
// Logic Layer (Sarait): registration, channels
//
// NOTE: MessageBuffer/BufferManager is separate (Variant 3),
// NOT stored inside Client
class Client {
public:
	// Constructor: initialize with file descriptor
	Client(int fd);

	// Destructor
	// ~Client();

	// ========== NETWORK LAYER (Alex) ==========
	int getFd() const;
	const std::string& getHostname() const;
	void setHostname(const std::string& hostname);

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
	// Network Layer
	int fd_;
	std::string hostname_;
    // std::string nickname_;
    // std::string username_;
    // std::string realname_;
    // std::string password_;
    
    // Registration flags
    // bool registered_;
    // bool passwordSet_;
    // bool nicknameSet_;
    // bool usernameSet_;
    
    // Channel membership
    // std::vector<std::string> channels_;

    // NOTE: NO MessageBuffer here (Variant 3)
};

#endif // CLIENT_HPP

