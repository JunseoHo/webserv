#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{
    version = "HTTP/1.1";
    statusCode = 200;
    message = "OK";
}

bool HttpResponse::configParse(const Server &server, const HttpRequest &request)
{
    std::string target = request.target;
    std::string version = request.version;
    if (std::list<Route>::const_iterator it = server.routes.begin(); it != server.routes.end(); it++)
    {
        if (it->location == target)
        {
            if (it->acceptedHttpMethods & request.method)
            {
                if (it->autoIndex)
                {
                    // autoindex
                }
                else
                {
                    // no autoindex
                }
            }
            else
            {
                // method not allowed
                statusCode = 405;
                message = "Method Not Allowed";
            }
        }
    }
    std::string first_line = version + " " + std::to_string(statusCode) + " " + message + "\r\n";
    headers["Content-Type"] = "text/plain";
    headers["Content-Length"] = "13";
    std::string body = "Hello, World!";
    response = first_line;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
    {
        response += it->first + ": " + it->second + "\r\n";
    }
    response += "\r\n" + body;
    return true;
}