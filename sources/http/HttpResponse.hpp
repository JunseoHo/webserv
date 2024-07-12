#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP
# include "Http.hpp"
# include "../config/Config.hpp"

class HttpResponse : public Http
{
	public:
		HttpResponse();
		HttpResponse(const HttpResponse& obj) {};
		~HttpResponse() {};

		HttpResponse& operator= (HttpResponse& rhs) { return *this; }

		std::string version;
		unsigned int statusCode;
		std::string message;
		std::string body;
		std::string response;

		virtual bool parse(const std::string &s) { return true; };
		bool configParse(const Server &server, const HttpRequest &request);
};

#endif