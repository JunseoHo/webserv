#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{
    version = "HTTP/1.1";
    statusCode = 200;
    message = "OK";
}

HttpResponse::HttpResponse(const std::string& cgiResponse)
{
	// 1. \r\n\r\n을 기준으로 S를 두 부분으로 나눈다.
    size_t delimiterPos = cgiResponse.find("\r\n\r\n");
    if (delimiterPos == std::string::npos) {
        throw std::invalid_argument("Input string does not contain two parts separated by \\r\\n\\r\\n");
    }

    std::string headerPart = cgiResponse.substr(0, delimiterPos);
    std::string bodyPart = cgiResponse.substr(delimiterPos + 4);

    // 2. 첫번째 부분을 라인 단위로 순회하는데 만약 :로 구분된 토큰의 왼쪽이 Status면 Http 응답의 스타트라인으로 수정한다.
    std::istringstream headerStream(headerPart);
    std::string line;
    std::string modifiedHeader;
    bool statusModified = false;
    bool isChunked = false;

    while (std::getline(headerStream, line)) {
        if (line.find("Status:") == 0 && !statusModified) {
            std::istringstream lineStream(line);
            std::string token;
            std::getline(lineStream, token, ':');  // "Status"
            std::getline(lineStream, token);      // " <status code and message>"
            modifiedHeader = "HTTP/1.1" + token + "\r\n" + modifiedHeader;
            statusModified = true;
        } 
		else if (line.find("Transfer-Encoding:") == 0) {
			std::istringstream lineStream(line);
            std::string token;
            std::getline(lineStream, token, ':');  // "Transfer-Encoding"
            std::getline(lineStream, token);      // " <chunked>"
            if (token.find("chunked") != std::string::npos)
            {
                isChunked = true;
                continue ;
            }
        }
		else {
            modifiedHeader += line + "\r\n";
        }
    }

    // 3. 두번째 부분은 문자열의 길이와 문자열의 내용이 번갈아 가면서 작성되어 있다. 내용을 하나로 합쳐라.
    std::string mergedBody;
    std::istringstream bodyStream(bodyPart);
    if (isChunked)
    {
        while (!bodyStream.eof()) {
            int len;
            bodyStream >> len;
            if (bodyStream.fail() || len <= 0) {
                break;
            }
            bodyStream.get();  // Consume the space after the length number
            char* buffer = new char[len];
            bodyStream.read(buffer, len);
            mergedBody.append(buffer, len);
            delete[] buffer;
        }
    }
    else
        mergedBody = bodyPart;
    
    // 4. 첫번째 부분의 결과에 두번째 내용의 길이를 기반으로 Content-Length 헤더를 추가하라.
    std::ostringstream contentLengthHeader;
    contentLengthHeader << "Content-Length: " << mergedBody.length() << "\r\n";
    modifiedHeader += contentLengthHeader.str();

    // 5. 두 부분을 다시 \r\n\r\n을 기준으로 결합하여 반환하라.
    std::string finalResult = modifiedHeader + "\r\n" + mergedBody;
	std::cout << finalResult << std::endl;
    full = finalResult;
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
    else if (statusCode != 201)
        body = readFileToString(uri);
    
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