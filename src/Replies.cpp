// Replies implementation
// Builds IRC numeric replies and formatted messages
// All messages end with \r\n (CRLF)

#include "irc/Replies.hpp"
#include <sstream>
#include <cstring>

// TODO: Define numeric reply constants
// const std::string Replies::RPL_WELCOME = "001";
// const std::string Replies::RPL_YOURHOST = "002";
// const std::string Replies::RPL_CREATED = "003";
// const std::string Replies::RPL_MYINFO = "004";
// ... etc

// TODO: Define error reply constants
// const std::string Replies::ERR_NOSUCHNICK = "401";
// ... etc

// TODO: Implement Replies::numeric(...)
// Format: ":servername numeric nickname params :trailing\r\n"
// Example: ":irc.example.com 001 user :Welcome to the IRC Network\r\n"
// - Build string with proper formatting
// - Always append \r\n
// - Return formatted string

// TODO: Implement Replies::command(...)
// Format: ":prefix command params :trailing\r\n"
// Example: ":nick!user@host PRIVMSG #channel :Hello\r\n"
// - Build string with prefix if provided
// - Always append \r\n
// - Return formatted string

// TODO: Implement Replies::simple(...)
// Format: "command params :trailing\r\n"
// Example: "PING :server.name\r\n"
// - Build string without prefix
// - Always append \r\n
// - Return formatted string

// TODO: Implement Replies::formatClientPrefix(...)
// Format: "nick!user@host"
// - Concatenate nickname, '!', username, '@', hostname
// - Return formatted string

// TODO: Implement Replies::formatServerName(...)
// - Return server name with leading ':' if needed
// - Or return as-is (depends on usage context)

