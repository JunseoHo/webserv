#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP
# include "Http.hpp"
# include "../config/Config.hpp"

class HttpResponse : public Http
{
	public:
		HttpResponse();
		HttpResponse(const Server &server, const HttpRequest &requset, int statusCode);
		HttpResponse(const HttpResponse& obj) {};
		~HttpResponse() {};

		HttpResponse& operator= (HttpResponse& rhs) { return *this; }

		unsigned int statusCode;
		std::string message;
		std::string response;
};

#endif