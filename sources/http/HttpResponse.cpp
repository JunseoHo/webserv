#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{
    version = "HTTP/1.1";
    statusCode = 200;
    message = "OK";
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename.substr(1));
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

std::string listDirectory(const std::string& path) {
    // 경로 문자열의 첫 글자를 제거
    std::string modified_path = path.substr(1);

    DIR *dir = opendir(modified_path.c_str());
    if (dir == NULL) {
        std::cerr << "Error opening directory: " << std::strerror(errno) << std::endl;
        return "";
    }

    struct dirent *entry;
    std::string file_list;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore '.' and '..' entries
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            file_list += entry->d_name;
            file_list += "\n";
        }
    }

    closedir(dir);
    return file_list;
}

HttpResponse::HttpResponse(const std::string& uri, const HttpRequest &request, int statusCode)
{
    setResponse(uri, request, statusCode, "");
}


HttpResponse::HttpResponse(const std::string& uri, const HttpRequest &request, int statusCode, const std::string& queryString)
{
    setResponse(uri, request, statusCode, queryString);
}

void HttpResponse::setResponse(const std::string& uri, const HttpRequest &request, int statusCode, const std::string& queryString)
{
    version = request.version;
    this->statusCode = statusCode;
    switch (statusCode)
    {
    case 100:
        message = "Continue";
        break;
    case 101:
        message = "Switching Protocols";
        break;
    case 200:
        message = "OK";
        break;
    case 201:
        message = "Created";
        break;
    case 204:
        message = "No Content";
        break;
    case 301:
        message = "Moved Permanently";
        break;
    case 302:
        message = "Found";
        break;
    case 304:
        message = "Not Modified";
        break;
    case 400:
        message = "Bad Request";
        break;
    case 401:
        message = "Unauthorized";
        break;
    case 403:
        message = "Forbidden";
        break;
    case 404:
        message = "Not Found";
        break;
    case 405:
        message = "Method Not Allowed";
        break;
    case 408:
        message = "Request Timeout";
        break;
    case 409:
        message = "Conflict";
        break;
    case 413:
        message = "Payload Too Large";
        break;
    case 414:
        message = "URI Too Long";
        break;
    case 429:
        message = "Too Many Requests";
        break;
    case 500:
        message = "Internal Server Error";
        break;
    case 502:
        message = "Bad Gateway";
        break;
    case 503:
        message = "Service Unavailable";
        break;
    case 504:
        message = "Gateway Timeout";
        break;
    case 505:
        message = "HTTP Version Not Supported";
        break;
    default:
        message = "Unknown";
        break;
    }

    response = version + " " + std::to_string(statusCode) + " " + message + "\r\n";

    std::cout << "URL RESPOJSE:" << uri << std::endl;

	if (uri.back() == '/') {
        if (isDirectory(uri.substr(1)))
		body = listDirectory(uri);
        
    }
    // else if (statusCode != 201)
    // {
    //     if (queryString != "")
    //     {
    //         std::string phpScriptPath = "/opt/homebrew/bin/php-cgi";
    //         std::string queryStringEnv = "QUERY_STRING=" + queryString;
    //         std::string uriEnv = uri.substr(1);
    //         char *phpArgs[] = {(char*)"php-cgi", (char*)(uriEnv.c_str()), NULL};
    //         char *envp[] = {(char*)queryStringEnv.c_str(), NULL};
    //         int pipefd[2];
    //         pipe(pipefd);
    //         pid_t pid = fork();
    //         if (pid == 0)
    //         {
    //             close(pipefd[0]);
    //             dup2(pipefd[1], 1);
    //             close(pipefd[1]);
    //             execve(phpScriptPath.c_str(), phpArgs, envp);
    //             exit(0);
    //         }
    //         close(pipefd[1]);
    //         char buffer[1024];
    //         int bytesRead;
    //         while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
    //             response += std::string(buffer, bytesRead);
    //         close(pipefd[0]);
    //         waitpid(pid, NULL, 0);
    //         return ;
    //     }
    //     body = readFileToString(uri);
    // }
    
    headers["Content-Length"] = std::to_string(body.size());
    if (statusCode == 201)
        ;
    else if (endsWith(uri, ".svg"))
        headers["Content-Type"] = "image/svg+xml";
    else if (endsWith(uri, ".html"))
        headers["Content-Type"] = "text/html";
    else if (endsWith(uri, ".css"))
        headers["Content-Type"] = "text/css";
    else if (endsWith(uri, ".js"))
        headers["Content-Type"] = "text/javascript";
    else if (endsWith(uri, ".jpg"))
        headers["Content-Type"] = "image/jpeg";
    else if (endsWith(uri, ".png"))
        headers["Content-Type"] = "image/png";
    else if (endsWith(uri, ".gif"))
        headers["Content-Type"] = "image/gif";
    else if (endsWith(uri, ".ico"))
        headers["Content-Type"] = "image/x-icon";
    else
        headers["Content-Type"] = "text/plain";
    headers["Connection"] = "keep-alive";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
        response += it->first + ": " + it->second + "\r\n";
    response += "\r\n" + body;
}