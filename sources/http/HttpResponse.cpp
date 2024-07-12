#include "HttpResponse.hpp"
#include <string>

HttpResponse::HttpResponse()
{
    version = "HTTP/1.1";
    statusCode = 200;
    message = "OK";
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool HttpResponse::configParse(const Server &server, const HttpRequest &request)
{
    std::string target = request.target;
    std::string version = request.version;
    for (std::list<Route>::const_iterator it = server.routes.begin(); it != server.routes.end(); it++)
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
    if (endsWith(target, ".svg"))
        headers["Content-Type"] = "image/svg+xml";
    std::string body = readFileToString("resources" + target);
    headers["Content-Length"] = std::to_string(body.length());
    response = version + " " + std::to_string(statusCode) + " " + message + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
    {
        response += it->first + ": " + it->second + "\r\n";
    }
    response += "\r\n" + body;
    return true;
}