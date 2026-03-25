// BotCommands.cpp — Command dispatch, alert, response handlers

#include "irc/bot/BotCommands.hpp"
#include "irc/bot/BotCore.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <ctime>

BotCommands::BotCommands(BotCore* core, const std::string& channel,
                         const std::string& nick)
    : core_(core), channel_(channel), nick_(nick), start_time_(time(NULL))
{
}

BotCommands::~BotCommands()
{
}

bool BotCommands::parsePrivmsg(const std::string& line,
                                std::string&       sender,
                                std::string&       target,
                                std::string&       text)
{
    if (line.empty() || line[0] != ':')
        return false;

    size_t ex  = line.find('!');
    size_t sp1 = line.find(' ');
    size_t sp2 = (sp1 != std::string::npos) ? line.find(' ', sp1 + 1) : std::string::npos;
    size_t sp3 = (sp2 != std::string::npos) ? line.find(' ', sp2 + 1) : std::string::npos;
    size_t col = line.find(':', sp3);

    if (ex == std::string::npos || sp1 == std::string::npos ||
        sp2 == std::string::npos || sp3 == std::string::npos)
        return false;

    if (line.substr(sp1 + 1, sp2 - sp1 - 1) != "PRIVMSG")
        return false;

    sender = line.substr(1, ex - 1);
    target = line.substr(sp2 + 1, sp3 - sp2 - 1);
    text   = (col != std::string::npos) ? line.substr(col + 1) : "";

    if (!text.empty() && text[text.size() - 1] == '\r')
        text.erase(text.size() - 1);

    return true;
}

void BotCommands::dispatch(const std::string& line)
{
    std::string sender, target, text;
    if (parsePrivmsg(line, sender, target, text)) {
        handle(sender, target, text);
    }
}

void BotCommands::cmdPing(const std::string& reply)
{
    core_->sendRaw("PRIVMSG " + reply + " :pong!");
}

void BotCommands::cmdEcho(const std::string& reply, const std::string& text)
{
    if (text.size() > 6) {
        core_->sendRaw("PRIVMSG " + reply + " :" + text.substr(6));
    }
}

void BotCommands::cmdTime(const std::string& reply)
{
    time_t now = time(NULL);
    std::string time_str = ctime(&now);
    if (!time_str.empty() && time_str[time_str.size() - 1] == '\n')
        time_str.erase(time_str.size() - 1);
    core_->sendRaw("PRIVMSG " + reply + " :" + time_str);
}

void BotCommands::cmdUptime(const std::string& reply)
{
    time_t now = time(NULL);
    long uptime = static_cast<long>(difftime(now, start_time_));

    std::ostringstream oss;
    oss << uptime;
    std::string uptime_str = oss.str();

    core_->sendRaw("PRIVMSG " + reply + " :uptime: " + uptime_str + "s");
}

void BotCommands::cmdHelp(const std::string& reply)
{
    core_->sendRaw("PRIVMSG " + reply
        + " :commands: !ping !echo <text> !time !uptime !help | coming soon: !play");
}

void BotCommands::handle(const std::string& sender,
                          const std::string& target,
                          const std::string& text)
{
    std::string reply = (!target.empty() && target[0] == '#') ? target : sender;

    if (text == "!ping") {
        cmdPing(reply);
    } else if (text.size() > 6 && text.substr(0, 6) == "!echo ") {
        cmdEcho(reply, text);
    } else if (text == "!time") {
        cmdTime(reply);
    } else if (text == "!uptime") {
        cmdUptime(reply);
    } else if (text == "!help") {
        cmdHelp(reply);
    }
}

void BotCommands::sendAlert()
{
    core_->sendRaw("PRIVMSG " + channel_ + " :[PLAYBOT] Status OK");
}
