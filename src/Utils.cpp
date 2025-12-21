// Utils implementation
// Utility functions for string manipulation, validation, formatting

#include "irc/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

// TODO: Implement Utils::trim(const std::string& str)
// - Remove leading and trailing whitespace
// - Return trimmed string

// TODO: Implement Utils::toUpper(const std::string& str)
// - Convert string to uppercase
// - Use std::transform with ::toupper
// - Return uppercase string

// TODO: Implement Utils::toLower(const std::string& str)
// - Convert string to lowercase
// - Use std::transform with ::tolower
// - Return lowercase string

// TODO: Implement Utils::split(const std::string& str, char delimiter)
// - Split string by delimiter
// - Return vector of substrings
// - Use std::stringstream or manual iteration

// TODO: Implement Utils::isValidNickname(const std::string& nickname)
// - Check length (1-9 characters, typically)
// - Check characters: letters, numbers, special chars [\]^_`{|}-
// - Cannot start with number
// - Return true if valid

// TODO: Implement Utils::isValidChannelName(const std::string& channelName)
// - Must start with # or &
// - Valid channel name characters (letters, numbers, special chars)
// - Length limits
// - Return true if valid

// TODO: Implement Utils::isValidHostname(const std::string& hostname)
// - Basic validation (can be IP address or domain name)
// - Return true if valid

// TODO: Implement Utils::isChannelChar(char c)
// - Check if character is valid in channel name
// - Return true if valid

// TODO: Implement Utils::isChannelName(const std::string& name)
// - Check if name starts with # or &
// - Return true if it's a channel name

// TODO: Implement Utils::stringToInt(const std::string& str)
// - Convert string to integer
// - Handle errors (return 0 or throw exception)
// - Use std::stringstream or atoi

// TODO: Implement Utils::intToString(int value)
// - Convert integer to string
// - Use std::stringstream or std::to_string (C++11) - check C++98 compatibility
// - For C++98, use stringstream

// TODO: Implement Utils::logError(const std::string& message)
// - Log error message to stderr or log file
// - Format: timestamp + message

// TODO: Implement Utils::logDebug(const std::string& message)
// - Log debug message (if debug mode enabled)
// - Format: timestamp + message

// TODO: Implement Utils::getCurrentTime()
// - Get current time as formatted string
// - Use strftime or similar
// - Return formatted time string

// TODO: Implement Utils::getCurrentTimestamp()
// - Get current time as time_t
// - Use time() function
// - Return timestamp

