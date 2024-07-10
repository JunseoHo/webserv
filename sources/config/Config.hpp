#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <list>
# include "../http/HttpRequest.hpp"

struct Route
{
	std::string	location;
	int			acceptedHttpMethods;
	bool		autoIndex;
	std::string index;
};

struct Server
{
	std::string				host;
	int						port;
	std::list<std::string> 	names;
	int						clientMaxBodySize;
	std::string				root;
	std::list<Route>		routes;
	std::string errorPage;
};

class Config
{
	public:
		Config(std::string& configFilePath);
		~Config();


		Server& selectServer(HttpRequest& httpRequest) const;
		void	print(void) const;

		class InvalidConfigFormatException : public std::exception
		{
			virtual const char* what() const throw();
		};

		std::list<int> getPorts() const;

	private:
		Config();
		Config(const Config& other);
		Config&	operator=(const Config& rhs);

		std::list<Server> mServers;
};

#endif