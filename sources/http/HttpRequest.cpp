#include "HttpRequest.hpp"
#include <string>
#include <map>

HttpRequest::HttpRequest() : Http() {
}

HttpRequest::HttpRequest(const HttpRequest& obj) : Http(obj) {
    this->method = obj.method;
    this->target = obj.target;
    this->version = obj.version;
}

HttpRequest::~HttpRequest() {
}

HttpRequest& HttpRequest::operator= (const HttpRequest& rhs) {
    this->headers = rhs.headers;
    this->body = rhs.body;
    this->method = rhs.method;
    this->target = rhs.target;
    this->version = rhs.version;
    return *this;
}

bool HttpRequest::parse(const std::string& s) {
    std::string startLine = s.substr(0, s.find("\r\n"));
    std::string method = startLine.substr(0, startLine.find(" "));
    std::string target = startLine.substr(startLine.find(" ") + 1, startLine.find(" ", startLine.find(" ") + 1) - startLine.find(" ") - 1);
    std::string version = startLine.substr(startLine.find(" ", startLine.find(" ") + 1) + 1);
    std::string headers = s.substr(s.find("\r\n") + 2, s.find("\r\n\r\n") - s.find("\r\n") - 2);
    std::map<std::string, std::string> headersMap;
    while (headers.find("\r\n") != std::string::npos)
    {
        std::string header = headers.substr(0, headers.find("\r\n"));
        headersMap[header.substr(0, header.find(":"))] = header.substr(header.find(":") + 2);
        headers = headers.substr(headers.find("\r\n") + 2);
    }
    std::string body = s.substr(s.find("\r\n\r\n") + 4);

    if (method == "" || target == "" || version == "" || headers == "")
        return false;

    // default page
    if (target == "/")
        target = "/index.html";

    if (method == "GET")
        this->method = GET;
    else if (method == "POST")
        this->method = POST;
    else if (method == "DELETE")
        this->method = DELETE;
    else
        return false;
    this->headers = headersMap;
    this->body = body;
    this->target = target;
    this->version = version;

    return true;
}

std::string HttpRequest::getValue(const std::string& key) {
    return this->headers[key];
}