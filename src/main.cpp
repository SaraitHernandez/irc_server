// Main entry point for IRC server
// Parse command line arguments, create Config, start Server

#include "irc/Config.hpp"
#include "irc/Server.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    // TODO: Parse command line arguments
    // Expected: ./ircserv <port> <password>
    // Example: ./ircserv 6667 secret
    
    // TODO: Validate arguments (port must be valid, password required)
    
    // TODO: Create Config object with port and password
    // Config config = Config::parseArgs(argc, argv);
    
    // TODO: Create Server instance with config
    // Server server(config);
    
    // TODO: Start server (bind, listen, run main loop)
    // server.start();
    // server.run();
    
    // TODO: Error handling
    // try {
    //     server.start();
    //     server.run();
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     return 1;
    // }
    
    (void)argc;
    (void)argv;
    return 0;
}

