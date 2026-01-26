// Client implementation
// Network Layer: fd, hostname (Alex)
// Logic Layer: registration, channels (Sarait)
//
// NOTE: MessageBuffer exists as a separate module (Parser side).
// In current Variant 3 design, buffering is handled by BufferManager
// (Parser/Server), NOT stored inside Client.

#include "irc/Client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Network Layer initialization
// DONE: Implement Client::Client(int fd)
// - Initialize fd_
// - Initialize all strings as empty
// - Set all registration flags to false
// - (Buffer is handled externally by MessageBuffer/BufferManager module)

Client::Client(int fd) 
	: fd_(fd),
		hostname_("unknown"),
		// nickname_(""),
		// username_(""),
		// realname_(""),
		// passwordSet_(false),
		// nicknameSet_(false),
		// usernameSet_(false),
		// registered_(false),
		// registrationStep_(0),
		// passwordAttempts_(0) {}

// DONE: dtor
// - Cleanup if needed (buffer cleanup handled by MessageBuffer destructor)
Client::~Client() {}

// DONE: getFd() const, getHostname() const
int	Client::getFd() const {
	return fd_;
}

const std::string& Client::getHostname() const {
	return hostname_;
}

// DONE: setHostname
void Client::setHostname(const std::string& hostname) {
	hostname_ = hostname;
}

// TODO: Implement getters
// - getNickname() const 
// - getUsername() const
// - getRealname() const

// TODO: Implement setters
// - setNickname(const std::string& nickname)
//   - Validate nickname (use Utils::isValidNickname)
//   - Set nickname_ and nicknameSet_
// - setUsername(const std::string& username, const std::string& realname)
//   - Set username_, realname_, and usernameSet_

//   - Or get from socket using getpeername() and inet_ntoa()

// TODO: Implement registration state methods
// - isRegistered() const
// - hasPassword() const
// - hasNickname() const
// - hasUsername() const
// - setPassword(const std::string& password)
// - registerClient() - set registered_ to true when all required info is set

// TODO: Implement channel membership methods
// - isInChannel(const std::string& channelName) const
// - isOperator(const std::string& channelName) const - check with Channel
// - joinChannel(const std::string& channelName)
// - leaveChannel(const std::string& channelName)
// - getChannels() const

// NOTE (Variant 3):
// Buffer management is handled by MessageBuffer/BufferManager
// on the Parser/Server side, NOT inside Client.
// this is why here are no methods:
// appendToBuffer()/getBuffer()/clearBuffer() 

// TODO: Implement Client::getPrefix() const
// - Format: "nick!user@host"
// - Return formatted string

