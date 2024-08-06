#include "Config.hpp"

Server::Server() { /* DO NOTHING */ }
Config::Config() { /* DO NOTHING */ }
Config::~Config() { /* DO NOTHING */ }

Config::Config(const Config& other) {
	if (this != &other)
		*this = other;
}

Config::Config(const std::string &configFilePath)
{
	std::ifstream configFile(configFilePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("Could not open configuration file.");

	std::string line, key, value;
	Server* currentServer = NULL;
	Location* currentLocation = NULL;
	bool inLocationBlock = false;

	while (std::getline(configFile, line))
	{	
		std::istringstream iss(line);

		if (line.empty())
			continue;

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
			else if (key == "cgi_path")
				currentLocation->cgiPath = value;
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

Config& Config::operator=(const Config& rhs) {
	if (this != &rhs)
		_servers = rhs._servers;
	return *this;
}

const Server& Config::SelectProcessingServer(const std::string& host, int port) const
{
	Server* processingServer = NULL;
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		if (it->listen == port)
		{
			// 만약 아래 조건문이 참인 Server가 없을 경우, 여기서 할당된 서버(디폴트 서버)가 반환된다.
			if (processingServer == NULL)
				processingServer = const_cast<Server*>(&(*it));
			if (it->serverName == host)
				return const_cast<Server &>(*it);
		}
	}
	return const_cast<Server &>(*processingServer);
}

const Location& Config::FindOptimalLocation(const Server& server, const std::string& uri) const
{
	const Location* optimalLocation = NULL;
	for (std::list<Location>::const_iterator it = server.locations.begin(); it != server.locations.end(); ++it)
	{
		const Location& location = *it;
		if ((uri.find(location.path) == 0) 
			&& (optimalLocation == NULL || location.path.size() > optimalLocation->path.size()))
				optimalLocation = &location;
	}
	if (optimalLocation == NULL)
		return *(server.locations.begin());
	return *optimalLocation;
}

const std::vector<int> Config::GetAllListeningPorts() const
{
	std::vector<int> listeningPorts;
	for (std::list<Server>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		// 이미 한번 삽입된 포트는 중복하여 삽입하지 않는다.
		if (std::find(listeningPorts.begin(), listeningPorts.end(), it->listen) == listeningPorts.end())
			listeningPorts.push_back(it->listen);
	}
	return listeningPorts;
}

const std::list<Server> Config::GetServers(void) const
{
	return _servers;
}

const char* Config::InvalidConfigFormatException::what() const throw()
{
	return "Configuration file format is incorrect.";
}

std::string Config::trim(const std::string& str) const
{
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
    return trimmed;
}

std::ostream& operator<<(std::ostream& os, const Config& config)
{
	os << "***********************************************************************" << std::endl;
	os << "***********************      Configuration      ***********************" << std::endl;
	os << "***********************************************************************" << std::endl << std::endl;
	const std::list<Server> servers = config.GetServers();
	for (std::list<Server>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt)
	{
		os << "Server Name          : " << serverIt->serverName << std::endl;
		os << "Listen               : " << serverIt->listen << std::endl;
		os << "Client Max Body Size : " << serverIt->clientMaxBodySize << std::endl;
		os << "Error Page           : " << serverIt->errorPage << std::endl;
		os << "Root                 : " << serverIt->root << std::endl;
		os << "Location             : [" << std::endl;
		for (std::list<Location>::const_iterator locIt = serverIt->locations.begin(); locIt != serverIt->locations.end(); )
		{
			os << "\t" << "{" << std::endl;
			os << "\t\t" << "Path                  :" << locIt->path << std::endl;
			os << "\t\t" << "Index                 :" << locIt->index << std::endl;
			os << "\t\t" << "Auto Index            :" << locIt->autoIndex << std::endl;
			os << "\t\t" << "Accepted Http Methods :" << locIt->acceptedHttpMethods << std::endl;
			os << "\t\t" << "CGI Path              :" << locIt->cgiPath << std::endl;
			os << "\t" << "}";
			++locIt;
			if (locIt != serverIt->locations.end())
				os << ",";
			os << "\t" << std::endl;
		}
		os << "]" << std::endl;
		os << std::endl << "***********************************************************************" << std::endl << std::endl;
	}
	return os;
}