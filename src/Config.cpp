// Config implementation
// Manages server configuration settings

#include "irc/Config.hpp"
#include <iostream>
#include <cstdlib>

Config::Config(int port, const std::string& password)
	: port_(port), password_(password) {
}

int Config::getPort() const {
	return port_;
}

const std::string& Config::getPassword() const {
	return password_;
}

void Config::setPort(int port) {
	port_ = port;
}

void Config::setPassword(const std::string& password) {
	password_ = password;
}

Config Config::parseArgs(int argc, char** argv) {
	int port = 6667;  // Default IRC port
	std::string password = "";

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		
		if (arg == "-p" || arg == "--port") {
			if (i + 1 < argc) {
				port = atoi(argv[++i]);
			}
		}
		else if (arg == "-pass" || arg == "--password") {
			if (i + 1 < argc) {
				password = argv[++i];
			}
		}
	}

	return Config(port, password);
}
