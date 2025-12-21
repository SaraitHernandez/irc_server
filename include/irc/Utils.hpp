#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

// Utility functions for IRC server
// String manipulation, validation, formatting helpers
namespace Utils {
    // String manipulation
    // std::string trim(const std::string& str);
    // std::string toUpper(const std::string& str);
    // std::string toLower(const std::string& str);
    
    // Split string by delimiter
    // std::vector<std::string> split(const std::string& str, char delimiter);
    
    // Validation
    // bool isValidNickname(const std::string& nickname);
    // bool isValidChannelName(const std::string& channelName);
    // bool isValidHostname(const std::string& hostname);
    
    // IRC protocol helpers
    // bool isChannelChar(char c);
    // bool isChannelName(const std::string& name);  // Starts with # or &
    
    // Number conversion
    // int stringToInt(const std::string& str);
    // std::string intToString(int value);
    
    // Error handling
    // void logError(const std::string& message);
    // void logDebug(const std::string& message);
    
    // Time utilities
    // std::string getCurrentTime();
    // time_t getCurrentTimestamp();
}

#endif // UTILS_HPP

