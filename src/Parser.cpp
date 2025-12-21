// Parser implementation
// Parses IRC message format: [:<prefix>] <command> [params] [:<trailing>]\r\n
// Converts string messages into Command structure

#include "irc/Parser.hpp"
#include "irc/Command.hpp"
#include "irc/Utils.hpp"
#include <sstream>

// TODO: Implement Parser::Parser()
// - Initialize if needed

// TODO: Implement Parser::parse(const std::string& message, Command& cmd)
// Main parsing logic:
// 1. Store raw message in cmd.raw
// 2. Remove trailing \r\n if present
// 3. Tokenize message by spaces (but preserve :trailing as single token)
// 4. First token:
//    - If starts with ':', it's a prefix - parse prefix, next token is command
//    - Otherwise, it's the command
// 5. Remaining tokens until ':' are params
// 6. Token starting with ':' is trailing (remove the ':')
// 7. Return true on success, false on error

// TODO: Implement Parser::isValidFormat(const std::string& message)
// - Check basic format:
//   - Must contain at least one space (for command)
//   - Must end with \r\n
//   - Command must be valid (alphanumeric or special chars)
//   - Prefix format if present: "nick!user@host" or "nick" or "server"

// TODO: Implement Parser::tokenize(const std::string& message)
// - Split by spaces, but handle ':' specially:
//   - When ':' is found, everything after is trailing (single token)
//   - Preserve leading ':' in trailing token
// - Return vector of tokens

// TODO: Implement Parser::parsePrefix(const std::string& token, Command& cmd)
// - Remove leading ':'
// - Store in cmd.prefix
// - Validate format (optional)

// TODO: Implement Parser::parseCommand(const std::string& token, Command& cmd)
// - Convert to uppercase
// - Store in cmd.command
// - Validate command name

// TODO: Implement Parser::parseParams(...)
// - Add tokens to cmd.params
// - Stop when ':' token is found (that's trailing)

// TODO: Implement Parser::parseTrailing(const std::string& trailing, Command& cmd)
// - Remove leading ':'
// - Store in cmd.trailing

// TODO: Implement Parser::isValidCommand(const std::string& command) const
// - Check if command is alphanumeric or contains allowed special chars
// - Return true if valid

// TODO: Implement Parser::isValidPrefix(const std::string& prefix) const
// - Validate prefix format: "nick!user@host" or "nick" or server name
// - Return true if valid

