#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP
# include "Http.hpp"

enum EHttpMethod
{
	GET = 1 << 0,
	POST = 1 << 1,
	DELETE = 1 << 2
};

class HttpRequest : public Http
{
	public:
		HttpRequest();
		HttpRequest(const HttpRequest& obj);
		~HttpRequest();

		HttpRequest& operator= (const HttpRequest& rhs);

		EHttpMethod method;
		std::string target;
		std::string version;

		virtual bool parse(const std::string& s);
};

#endif