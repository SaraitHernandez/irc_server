#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
// #include "Command.hpp"

// Forward declaration
struct Command;

// Parser class - parses IRC message format into Command structure
// Format: [:<prefix>] <command> [params] [:<trailing>]\r\n
class Parser {
public:
    // Constructor
    // Parser();
    
    // Destructor
    // ~Parser();
    
    // Parse a complete IRC message string into Command structure
    // Returns true if parsing successful, false otherwise
    // bool parse(const std::string& message, Command& cmd);
    
    // Validate message format (basic validation)
    // static bool isValidFormat(const std::string& message);
    
    // Helper: split message into tokens
    // static std::vector<std::string> tokenize(const std::string& message);
    
private:
    // Helper methods for parsing different parts
    // bool parsePrefix(const std::string& token, Command& cmd);
    // bool parseCommand(const std::string& token, Command& cmd);
    // bool parseParams(const std::vector<std::string>& tokens, Command& cmd, size_t startIdx);
    // void parseTrailing(const std::string& trailing, Command& cmd);
    
    // bool isValidCommand(const std::string& command) const;
    // bool isValidPrefix(const std::string& prefix) const;
};

#endif // PARSER_HPP

