#ifndef HTTP_HPP
# define HTTP_HPP
# include <string>
# include <map>

class Http
{
	public:
		Http() {};
		Http(const Http& obj) {
			this->headers = obj.headers;
			this->body = obj.body;
		};
		~Http() {};

		Http& operator= (Http& rhs) { return *this; };

		std::map<std::string, std::string> headers;
		std::string body;

		virtual bool parse(const std::string& s) = 0;
};

#endif