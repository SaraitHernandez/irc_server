#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

// Replies class - builds IRC numeric replies and formatted messages
// All messages end with \r\n (CRLF)
// See TEAM_CONVENTIONS.md: Message formatting
class Replies {
public:
    // Numeric reply constants (RFC 2812)
    // static const std::string RPL_WELCOME;           // 001
    // static const std::string RPL_YOURHOST;          // 002
    // static const std::string RPL_CREATED;           // 003
    // static const std::string RPL_MYINFO;            // 004
    // static const std::string RPL_TOPIC;             // 332
    // static const std::string RPL_NAMREPLY;          // 353
    // static const std::string RPL_ENDOFNAMES;        // 366
    
    // Error reply constants
    // static const std::string ERR_NOSUCHNICK;        // 401
    // static const std::string ERR_NOSUCHCHANNEL;     // 403
    // static const std::string ERR_CANNOTSENDTOCHAN;  // 404
    // static const std::string ERR_UNKNOWNCOMMAND;    // 421
    // static const std::string ERR_NONICKNAMEGIVEN;   // 431
    // static const std::string ERR_ERRONEUSNICKNAME;  // 432
    // static const std::string ERR_NICKNAMEINUSE;     // 433
    // static const std::string ERR_NICKCOLLISION;     // 436
    // static const std::string ERR_NOTONCHANNEL;      // 442
    // static const std::string ERR_NOTREGISTERED;     // 451
    // static const std::string ERR_NEEDMOREPARAMS;    // 461
    // static const std::string ERR_ALREADYREGISTRED;  // 462
    // static const std::string ERR_PASSWDMISMATCH;    // 464
    // static const std::string ERR_CHANNELISFULL;     // 471
    // static const std::string ERR_UNKNOWNMODE;       // 472
    // static const std::string ERR_INVITEONLYCHAN;    // 473
    // static const std::string ERR_BADCHANNELKEY;     // 475
    // static const std::string ERR_CHANOPRIVSNEEDED;  // 482
    
    // Build numeric reply: ":server numeric nickname params :trailing\r\n"
    // static std::string numeric(const std::string& numeric,
    //                            const std::string& serverName,
    //                            const std::string& nickname,
    //                            const std::string& params = "",
    //                            const std::string& trailing = "");
    
    // Build command message: ":prefix command params :trailing\r\n"
    // static std::string command(const std::string& prefix,
    //                            const std::string& command,
    //                            const std::string& params = "",
    //                            const std::string& trailing = "");
    
    // Build simple command (no prefix): "command params :trailing\r\n"
    // static std::string simple(const std::string& command,
    //                           const std::string& params = "",
    //                           const std::string& trailing = "");
    
    // Format client prefix: "nick!user@host"
    // static std::string formatClientPrefix(const std::string& nick,
    //                                       const std::string& user,
    //                                       const std::string& host);
    
    // Format server name (usually from config)
    // static std::string formatServerName(const std::string& name);
};

#endif // REPLIES_HPP

