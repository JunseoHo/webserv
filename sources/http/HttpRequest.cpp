#include "HttpRequest.hpp"
#include <string>
#include <map>
#include <iostream>

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
    
    // 헤더와 바디 분리
    std::string headerAndBody = s.substr(s.find("\r\n") + 2);
    std::string header = headerAndBody.substr(0, headerAndBody.find("\r\n\r\n") + 2);
    std::string body = headerAndBody.substr(headerAndBody.find("\r\n\r\n") + 4);
    this->body = body;

    // 헤더 파싱
    size_t pos = 0;
    while ((pos = header.find("\r\n")) != std::string::npos) {
        std::string line = header.substr(0, pos);
        header.erase(0, pos + 2);
        std::string key = line.substr(0, line.find(":"));
        std::string value = line.substr(line.find(":") + 2);
        this->headers[key] = value;
    }

    if (target == "" || version == "")
        return false;

    return true;
}

std::string HttpRequest::getValue(const std::string& key) {
    return this->headers[key];
}