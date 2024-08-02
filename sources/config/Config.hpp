#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <list>
# include <vector>
# include "../http/HttpRequest.hpp"

struct Location
{
	std::string	path;
	int			acceptedHttpMethods;
	bool		autoIndex;
	std::string index;
	std::string cgiPath;
};

struct Server
{
	Server();
	int						listen;
	std::string 			serverName;
	int						clientMaxBodySize;
	std::string				root;
	std::list<Location>		locations;
	std::string				errorPage;
};

class Config
{
	public:
		Config();
		Config(const Config& other);
		Config(const std::string& configFilePath);
		
		Config&	operator=(const Config& rhs);
		
		~Config();

		const Server&			SelectProcessingServer(const std::string& host, int port) const;
		const Location&			FindOptimalLocation(const Server& server, const std::string& uri) const;
		const std::vector<int>	GetAllListeningPorts(void) const;
		const std::list<Server> GetServers(void) const;

		class InvalidConfigFormatException : public std::exception
		{
			virtual const char* what() const throw();
		};

	private:
		std::list<Server> _servers;

		std::string trim(const std::string& str) const;
};

std::ostream& operator<<(std::ostream& os, const Config& config);
#endif