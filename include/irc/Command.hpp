#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

// Command structure - parsed IRC message
// This is the format used to pass parsed messages between modules
// See TEAM_CONVENTIONS.md for details
struct Command {
    // Optional origin prefix (e.g., "nick!user@host")
    std::string prefix;
    
    // Command name (e.g., "NICK", "PRIVMSG", "JOIN")
    std::string command;
    
    // Command parameters (space-separated, before trailing)
    std::vector<std::string> params;
    
    // Trailing parameter (content after ':')
    std::string trailing;
    
    // Original unparsed message (for debugging/logging)
    std::string raw;
    
    // Helper methods (can be implemented as free functions or in Utils)
    // bool hasPrefix() const { return !prefix.empty(); }
    // std::string getParam(size_t index) const;
    // size_t paramCount() const { return params.size(); }
    // std::string getFullCommand() const;  // Reconstruct command string
};

#endif // COMMAND_HPP

