#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP
# include "../http/Http.hpp"

enum EHttpMethod
{
	UNKNOWN = 0,
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

		void parse(const std::string& s);
		std::string getValue(const std::string& key);
};

#endif