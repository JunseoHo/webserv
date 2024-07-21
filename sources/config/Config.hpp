#ifndef CONFIG_HPP
# define CONFIG_HPP

# define ON 0
# define OFF 1
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
	int			acceptedHttpMethods = 7;
	bool		autoIndex;
	std::string index;
};

struct Server
{
	Server();
	int						listen;
	std::string 			serverName;
	int						clientMaxBodySize;
	std::string				root;
	std::list<Location>		locations;
	std::string errorPage;
};

class Config
{
	public:
		Config();
		Config(std::string& configFilePath);
		Config&	operator=(const Config& rhs);
		Config(const Config& other);
		~Config();


		Server& selectServer(HttpRequest& httpRequest, int port) const;
		const Location& findLocation(const Server& server, const std::string& uri) const;
		void	print(void) const;

		class InvalidConfigFormatException : public std::exception
		{
			virtual const char* what() const throw();
		};

		std::vector<int> getPorts() const;

	private:
		std::list<Server> _servers;
};

#endif