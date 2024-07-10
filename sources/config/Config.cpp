#include "Config.hpp"

Config::Config() { /* DO NOTHING */ }
Config::~Config() { /* DO NOTHING */ }
Config::Config(const Config& other) { /* DO NOTHING */ }

int parseHttpMethods(const std::string& methods) {
    int result = 0;
    if (methods.find("GET") != std::string::npos) result |= 1;
    if (methods.find("POST") != std::string::npos) result |= 2;
    if (methods.find("DELETE") != std::string::npos) result |= 4;
    return result;
}

void parseServerBlock(std::ifstream &file, Server &server) {
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		iss >> key;

		if (key == "listen:")
			iss >> server.port;
		else if (key == "server_name:")
		{
			std::string name;
			while (iss >> name)
				server.names.push_back(name);
		}
		else if (key == "client_max_body_size:")
			iss >> server.clientMaxBodySize;
		else if (key == "root:")
			iss >> server.root;
		else if (key == "location:")
		{
			Route route;
			iss >> route.location;
			while (std::getline(file, line) && line.find("location:") == std::string::npos)
			{
				std::istringstream loc_iss(line);
				loc_iss >> key;
				if (key == "allow_methods:")
				{
					std::string methods;
					loc_iss >> methods;
					route.acceptedHttpMethods = parseHttpMethods(methods);
				}
				else if (key == "autoindex:")
				{
					std::string value;
					loc_iss >> value;
					route.autoIndex = (value == "on");
				}
				else if (key == "root:")
					loc_iss >> route.index;
				else if (key == "client_error_page:")
					loc_iss >> route.errorPage;
			}
			server.routes.push_back(route);
			if (line.find("location:") != std::string::npos)
			{
				std::istringstream new_iss(line);
				new_iss >> key >> route.location;
			}
		}
	}
}

Config::Config(std::string& configFilePath)
{
	std::ifstream file(configFilePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open configuration file.");
	
	std::string line;
	while (std::getline(file, line)) {
		if (line.find("server:") != std::string::npos) {
			Server server;
			parseServerBlock(file, server);
			mServers.push_back(server);
		}
	}
}

const char* Config::InvalidConfigFormatException::what() const throw()
{
	return "Configuration file format is incorrect.";
}