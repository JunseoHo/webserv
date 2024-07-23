#include "Config.hpp"

Server::Server() { /* DO NOTHING */ }

Config::Config() { /* DO NOTHING */ }
Config::~Config() { /* DO NOTHING */ }

Config &Config::operator=(const Config& rhs) {
	if (this != &rhs)
		_servers = rhs._servers;
	return *this;
}

Config::Config(const Config& other) {
	if (this != &other)
	*this = other;
}

std::string trim(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
    return trimmed;
}

Config::Config(std::string &configFilePath)
{
	std::ifstream configFile(configFilePath.c_str());

	if (!configFile.is_open())
		throw std::runtime_error("Could not open configuration file."); // 프로그램이 중단되지 않고 있음

	std::string line;
	std::string key, value;

	Server* currentServer = NULL;
	Location* currentLocation = NULL;
	
	bool inLocationBlock = false;

	while (std::getline(configFile, line))
	{	
		std::istringstream iss(line);

		if (line.empty()) continue;

		if (line.find(':') != std::string::npos)
		{
			std::size_t pos = line.find(':');

			key = trim(line.substr(0, pos));
            value = trim(line.substr(pos + 1));

			if (key == "server")
			{
				if (currentLocation != NULL)
				{
					currentServer->locations.push_back(*currentLocation);
					delete currentLocation;
					currentLocation = NULL;
				}
				if (currentServer != NULL)
				{
					_servers.push_back(*currentServer);
					delete currentServer;
				}
				currentServer = new Server();
			}
			else if (key == "listen")
				currentServer->listen = std::stoi(value);
			else if (key == "server_name")
				currentServer->serverName = value;
			else if (key == "error_page")
				currentServer->errorPage = value;
			else if (key == "client_max_body_size")
				currentServer->clientMaxBodySize = std::stoi(value);
			else if (key == "root")
				currentServer->root = value;
			else if (key == "path")
				currentLocation->path = value;
			else if (key == "allow_methods")
			{
				if (value.find("GET") != std::string::npos)
					currentLocation->acceptedHttpMethods |= 1;
				if (value.find("POST") != std::string::npos)
					currentLocation->acceptedHttpMethods |= 2;
				if (value.find("DELETE") != std::string::npos)
					currentLocation->acceptedHttpMethods |= 4;
			}
			else if (key == "index")
				currentLocation->index = value;
			else if (key == "autoindex")
				currentLocation->autoIndex = (value == "on");
			else if (key == "location")
			{
				if (currentLocation != NULL)
				{
					currentServer->locations.push_back(*currentLocation);
					delete currentLocation;
				}
				currentLocation = new Location();
				currentLocation->path = value;
			}
		}
		else
			throw std::runtime_error("Invalid format.");
	}

	if (currentLocation != NULL)
	{
		currentServer->locations.push_back(*currentLocation);
		delete currentLocation;
	}
	if (currentServer != NULL)
	{
		_servers.push_back(*currentServer);
		delete currentServer;
	}
	configFile.close();
}

const char* Config::InvalidConfigFormatException::what() const throw()
{
	return "Configuration file format is incorrect.";
}

void Config::print(void) const
{
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		std::cout << "[Server Instance]\n";
	    std::cout << "\tListen: " << it->listen << "\n";
	    std::cout << "\tServer Name: " << it->serverName << "\n";
	    std::cout << "\tError Page: " << it->errorPage << "\n";
	    std::cout << "\tClient Max Body Size: " << it->clientMaxBodySize << "\n";
	    std::cout << "\tRoot: " << it->root << "\n";
		it->locations.begin();
	    for (std::list<Location>::const_iterator it2 = it->locations.begin(); it2 != it->locations.end(); ++it2) {
	        const Location& location = *it2;
			std::cout << "\tLocation:\n";
			std::cout << "\t\tAccepted HTTP Methods: " << location.acceptedHttpMethods << "\n";
	        std::cout << "\t\tPath: " << location.path << "\n";
	        std::cout << "\t\tIndex: " << location.index << "\n";
	        std::cout << "\t\tAutoindex: " << (location.autoIndex ? "on" : "off") << "\n";
	    }
		std::cout << "\n";
	}
}

std::vector<int> Config::getPorts() const {
	std::vector<int> ports;
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
		ports.push_back(it->listen);
	return ports;
}

Server& Config::selectServer(HttpRequest& httpRequest, int port) const {
	Server* defaultServer = NULL;
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		if (it->listen == port)
		{
			if (defaultServer == NULL)
				defaultServer = const_cast<Server*>(&(*it));
			if (it->serverName == httpRequest.getValue("Host"))
				return const_cast<Server &>(*it);	
		}
	}
	return const_cast<Server &>(*defaultServer);
}

const Location& Config::findLocation(const Server& server, const std::string& uri) const {
	// 가장 길게 매칭되는 location을 찾아서 반환
	const Location* longestMatch = NULL;
	for (std::list<Location>::const_iterator it = server.locations.begin(); it != server.locations.end(); ++it)
	{
		const Location& location = *it;
		if (uri.find(location.path) == 0)
		{
			if (longestMatch == NULL || location.path.size() > longestMatch->path.size())
				longestMatch = &location;
		}
	}
	if (longestMatch == NULL)
		return *(server.locations.begin());
	return *longestMatch;
}