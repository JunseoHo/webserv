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
			this->version = obj.version;
		};
		~Http() {};

		Http& operator= (Http& rhs) {
			if (this == &rhs)
				return *this;
			this->headers = rhs.headers;
			this->body = rhs.body;
			this->version = rhs.version;
			return *this;
		};

		std::map<std::string, std::string> headers;
		std::string body;
		std::string version;
		std::string referer;
};

#endif