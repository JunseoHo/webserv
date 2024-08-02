#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP
# include "Http.hpp"
# include "../config/Config.hpp"
# include <string>
# include <unistd.h>
# include <iostream>
# include <dirent.h>
# include <sys/types.h>
# include <cerrno>
# include <cstring>
# include "../utils/utils.h"

class HttpResponse : public Http
{
	public:
		HttpResponse();
		HttpResponse(const std::string& uri, const HttpRequest &requset, int statusCode);
		HttpResponse(const std::string& uri, const HttpRequest &requset, int statusCode, const std::string& queryString);
		HttpResponse(const std::string& cgiResponse);
		HttpResponse(const HttpResponse& obj) {};
		~HttpResponse() {};

		void setResponse(const std::string& uri, const HttpRequest &requset, int statusCode, const std::string& queryString);

		HttpResponse& operator= (HttpResponse& rhs) { return *this; }

		unsigned int statusCode;
		std::string message;
		std::string response;
};

#endif