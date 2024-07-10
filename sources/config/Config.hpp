#ifndef SERVER_HPP
# define SERVER_HPP
# include <string>
# include <list>
# include "../http/HttpRequest.hpp"

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

struct Route
{
	std::string	location;
	int			acceptedHttpMethods;
	bool		autoIndex;
	std::string index;
};

class Config
{
	public:
		Config(std::string& configFilePath);
		~Config();

		Config&	operator=(const Config& rhs);

		Server& selectServer(HttpRequest& httpRequest) const;
		void	print(void) const;

		class InvalidConfigFormatException : public std::exception
		{
			virtual const char* what() const throw();
		};

	private:
		Config();
		Config(const Config& other);

		std::list<Server> mServers;
};

#endif