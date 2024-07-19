#include "Config.hpp"

Server::Server() : status(OFF) {}

Config::Config() { /* DO NOTHING */ }
Config::~Config() { /* DO NOTHING */ }

Config &Config::operator=(const Config& rhs) {
	if (this != &rhs)
		mServers = rhs.mServers;
	return *this;
}

Config::Config(const Config& other) {
	if (this != &other)
	*this = other;
}

Config::Config(std::string &configFilePath)
{
	std::ifstream file(configFilePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open configuration file.");

	std::string line;
	Server currentServer;
	Route currentRoute;
	bool inServerBlock = false;
	bool inLocationBlock = false;

	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key;
		if (line.find("server:") != std::string::npos)
		{
			if (inServerBlock)
			{
				currentServer.status = ON;
				mServers.push_back(currentServer);
				currentServer = Server();
			}
			inServerBlock = true;
			inLocationBlock = false;
		}
		else if (line.find("location:") != std::string::npos)
		{
			if (inLocationBlock)
			{
				currentServer.routes.push_back(currentRoute);
				currentRoute = Route();
			}
			inLocationBlock = true;
			iss >> key >> currentRoute.location;
		}
		else if (inServerBlock)
		{
			iss >> key;
			if (key == "listen:")
			{
				iss >> currentServer.port;
			}
			else if (key == "server_name:")
			{
				std::string name;
				while (iss >> name)
					currentServer.names.push_back(name);
			}
			else if (key == "host:")
				iss >> currentServer.host;
			else if (key == "client_max_body_size:")
				iss >> currentServer.clientMaxBodySize;
			else if (key == "root:")
				iss >> currentServer.root;
			else if (key == "error_page:")
				iss >> currentServer.errorPage;
			else if (inLocationBlock)
			{
				if (key == "allow_methods:")
				{
					std::string method;
					while (iss >> method)
					{
						if (method == "GET")
							currentRoute.acceptedHttpMethods |= 1 << 0;
						else if (method == "POST")
							currentRoute.acceptedHttpMethods |= 1 << 1;
						else if (method == "DELETE")
							currentRoute.acceptedHttpMethods |= 1 << 2;
					}
				}
				else if (key == "autoindex:")
				{
					std::string value;
					iss >> value;
					currentRoute.autoIndex = (value == "on");
				}
				else if (key == "index:")
					iss >> currentRoute.index;
			}
		}
	}
	if (inServerBlock) {
		currentServer.status = ON;
		mServers.push_back(currentServer);
	}
	file.close();
}

const char* Config::InvalidConfigFormatException::what() const throw()
{
	return "Configuration file format is incorrect.";
}

void Config::print(void) const
{
	for (std::list<Server>::const_iterator it = mServers.begin(); it != mServers.end(); ++it)
	{
		std::cout << "Server: " << it->port << std::endl;
		std::cout << "Names: ";
		for (std::list<std::string>::const_iterator it2 = it->names.begin(); it2 != it->names.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
		std::cout << "Host: " << it->host << std::endl;
		std::cout << "Client max body size: " << it->clientMaxBodySize << std::endl;
		std::cout << "Root: " << it->root << std::endl;
		std::cout << "Error page: " << it->errorPage << std::endl;
		for (std::list<Route>::const_iterator it2 = it->routes.begin(); it2 != it->routes.end(); ++it2)
		{
			std::cout << "Route: " << it2->location << std::endl;
			std::cout << "Accepted methods: ";
			if (it2->acceptedHttpMethods & 1 << 0)
				std::cout << "GET ";
			if (it2->acceptedHttpMethods & 1 << 1)
				std::cout << "POST ";
			if (it2->acceptedHttpMethods & 1 << 2)
				std::cout << "DELETE ";
			std::cout << std::endl;
			std::cout << "Autoindex: " << (it2->autoIndex ? "on" : "off") << std::endl;
			std::cout << "Index: " << it2->index << std::endl;
		}
	}
}

std::vector<int> Config::getPorts() const {
	std::vector<int> ports;
	for (std::list<Server>::const_iterator it = mServers.begin(); it != mServers.end(); ++it)
		ports.push_back(it->port);
	return ports;
}

Server& Config::selectServer(HttpRequest& httpRequest, int port) const {
	Server* defaultServer = NULL;
	for (std::list<Server>::const_iterator it = mServers.begin(); it != mServers.end(); ++it)
	{
		if (it->port == port)
		{
			if (defaultServer == NULL)
				defaultServer = const_cast<Server*>(&(*it));
			if (it->host == httpRequest.getValue("Host"))
				return const_cast<Server &>(*it);	
		}
	}
	return const_cast<Server &>(*defaultServer);
}