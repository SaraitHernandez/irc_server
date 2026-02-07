#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <ctime>

// Utility functions for IRC server
// String manipulation, validation, formatting helpers
namespace Utils {
    // String manipulation
    std::string trim(const std::string& str);
    std::string toUpper(const std::string& str);
    std::string toLower(const std::string& str);
    
    // Split string by delimiter
    std::vector<std::string> split(const std::string& str, char delimiter);
    
    // Validation (SIMPLIFIED rules per TEAM_CONVENTIONS.md)
    // Nickname: 1-9 chars, first char letter/underscore, rest letters/digits/underscore
    bool isValidNickname(const std::string& nickname);
    // Channel: starts with #, 2-50 chars, no space/comma/control chars
    bool isValidChannelName(const std::string& channelName);
    
    // IRC protocol helpers
    bool isChannelName(const std::string& name);  // Starts with #
    
    // Number conversion (C++98 compatible)
    int stringToInt(const std::string& str);
    std::string intToString(int value);
    
    // Time utilities
    std::string getCurrentTime();
    time_t getCurrentTimestamp();
}

#endif // UTILS_HPP
