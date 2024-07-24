#include "HttpRequest.hpp"
#include <string>
#include <map>

HttpRequest::HttpRequest() : Http() {
}

HttpRequest::HttpRequest(const HttpRequest& obj) : Http(obj) {
    this->method = obj.method;
    this->target = obj.target;
}

HttpRequest::~HttpRequest() {
}

HttpRequest& HttpRequest::operator= (const HttpRequest& rhs) {
    if (this == &rhs)
        return *this;
    this->method = rhs.method;
    this->target = rhs.target;
    this->headers = rhs.headers;
    this->body = rhs.body;
    this->version = rhs.version;
    return *this;
}

std::string HttpRequest::parseReferer(const std::string& content) {
    std::string refererHeader = "Referer: ";
    std::string refererPath = "";

    // Find the "Referer: " line
    size_t pos = content.find(refererHeader);
    if (pos != std::string::npos) {
        // Extract the URL after "Referer: "
        size_t start = pos + refererHeader.length();
        size_t end = content.find("\n", start);
        std::string refererUrl = content.substr(start, end - start);

        // Find the path part by locating the third '/'
        size_t pathPos = refererUrl.find('/', refererUrl.find('/', refererUrl.find('/') + 1) + 1);
        if (pathPos != std::string::npos) {
            refererPath = refererUrl.substr(pathPos);
        }
    }

    return refererPath;
}

bool HttpRequest::parse(const std::string& s) {
    std::string startLine = s.substr(0, s.find("\r\n"));
    std::string method = startLine.substr(0, startLine.find(" "));
    if (method == "GET")
        this->method = GET;
    else if (method == "POST")
        this->method = POST;
    else if (method == "DELETE")
        this->method = DELETE;
    else
        return false;
    target = startLine.substr(startLine.find(" ") + 1, startLine.find(" ", startLine.find(" ") + 1) - startLine.find(" ") - 1);
    version = startLine.substr(startLine.find(" ", startLine.find(" ") + 1) + 1);
    referer = parseReferer(s);
    std::string fullHeader = s.substr(s.find("\r\n") + 2, s.find("\r\n\r\n") - s.find("\r\n") - 2);
    while (fullHeader.find("\r\n") != std::string::npos)
    {
        std::string header = fullHeader.substr(0, fullHeader.find("\r\n"));
        headers[header.substr(0, header.find(":"))] = header.substr(header.find(":") + 2);
        fullHeader = fullHeader.substr(fullHeader.find("\r\n") + 2);
    }
    body = s.substr(s.find("\r\n\r\n") + 4);

    if (target == "" || version == "")
        return false;

    return true;
}

std::string HttpRequest::getValue(const std::string& key) {
    return this->headers[key];
}