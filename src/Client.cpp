// Client implementation
// Manages client state, registration, and message buffer

#include "irc/Client.hpp"
#include "irc/MessageBuffer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// TODO: Implement Client::Client(int fd)
// - Initialize fd_
// - Initialize all strings as empty
// - Set all registration flags to false
// - Initialize MessageBuffer

// TODO: Implement Client::~Client()
// - Cleanup if needed (buffer cleanup handled by MessageBuffer destructor)

// TODO: Implement getters
// - getFd() const
// - getNickname() const
// - getUsername() const
// - getHostname() const
// - getRealname() const

// TODO: Implement setters
// - setNickname(const std::string& nickname)
//   - Validate nickname (use Utils::isValidNickname)
//   - Set nickname_ and nicknameSet_
// - setUsername(const std::string& username, const std::string& realname)
//   - Set username_, realname_, and usernameSet_
// - setHostname(const std::string& hostname)
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

// TODO: Implement buffer management
// - appendToBuffer(const std::string& data) - delegate to MessageBuffer
// - getBuffer() - get buffer string from MessageBuffer
// - clearBuffer() - clear MessageBuffer
// - getMessageBuffer() - return reference to MessageBuffer

// TODO: Implement Client::getPrefix() const
// - Format: "nick!user@host"
// - Return formatted string

