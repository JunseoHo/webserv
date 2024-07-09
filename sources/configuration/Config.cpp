#include "Config.hpp"
Config::Config() { /* DO NOTHING */ }
Config::~Config() { /* DO NOTHING */ }
Config::Config(Config const &src) { /* DO NOTHING */ }
Config &Config::operator=(Config const &rhs) { /* DO NOTHING */ }

Config::Config(char *conf) { readConfigFile(conf); }

bool Config::readConfigFile(char *filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open config file" << std::endl;
		return false;
	}

	std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.find("listen:") == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string portStr = line.substr(colonPos + 1);
                portStr.erase(0, portStr.find_first_not_of(" \t"));
                portStr.erase(portStr.find_last_not_of(" \t") + 1);
                int port = std::atoi(portStr.c_str());
                ports.push_back(port);
            }
        }
    }
    file.close();
    return true;
}

std::list<int> Config::getPorts() const { return ports; }