// BotMain.cpp — Main event loop: connect, poll, dispatch, alert

#include "irc/bot/BotCore.hpp"
#include "irc/bot/BotCommands.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#define ALERT_INTERVAL 60  // Send status alert every 60 seconds

int main(int argc, char* argv[])
{
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0]
                  << " <host> <port> <pass> <nick> <channel>" << std::endl;
        std::cerr << "Example: " << argv[0]
                  << " 127.0.0.1 6667 testpass playbot #general" << std::endl;
        return 1;
    }

    std::string host    = argv[1];
    int         port    = std::atoi(argv[2]);
    std::string pass    = argv[3];
    std::string nick    = argv[4];
    std::string channel = argv[5];

    BotCore core(host, port, pass, nick, channel);
    BotCommands cmds(&core, channel, nick);

    if (!core.connect()) {
        std::cerr << "[ERR] Failed to connect" << std::endl;
        return 1;
    }

    if (!core.registerIRC()) {
        std::cerr << "[ERR] Failed to register" << std::endl;
        return 1;
    }

    std::cout << "[BOT] Entering main loop (ALERT_INTERVAL=" << ALERT_INTERVAL
              << "s)" << std::endl;

    time_t last_alert = time(NULL);

    while (true) {
        time_t now = time(NULL);
        long elapsed = static_cast<long>(difftime(now, last_alert));
        int timeout_ms = ((ALERT_INTERVAL - elapsed) > 0)
                       ? (ALERT_INTERVAL - elapsed) * 1000
                       : 0;

        std::vector<std::string> lines = core.tick(timeout_ms);

        for (size_t i = 0; i < lines.size(); ++i) {
            std::cout << "[BOT<<] " << lines[i] << std::endl;
            cmds.dispatch(lines[i]);
        }

        // Send alert if poll() timeout expired
        now = time(NULL);
        elapsed = static_cast<long>(difftime(now, last_alert));
        if (elapsed >= ALERT_INTERVAL) {
            cmds.sendAlert();
            last_alert = now;
        }

        // Reconnect if lost connection
        if (!core.isConnected()) {
            std::cout << "[BOT] Connection lost, attempting reconnect..." << std::endl;
            if (!core.reconnect(5)) {
                std::cerr << "[BOT] Failed to reconnect, exiting" << std::endl;
                return 1;
            }
            last_alert = time(NULL);  // Reset alert timer after reconnect
        }
    }

    return 0;
}
