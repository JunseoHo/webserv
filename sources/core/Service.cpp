#include "Service.hpp"
const int Service::_backLog = 10;
Service::Service() { /* DO NOTHING */ }
Service::~Service() { /* DO NOTHING */ }

Service::Service(const Service& obj) { 
    if (this != &obj)
        *this = obj;
}

Service& Service::operator= (const Service& rhs) {
    if (this != &rhs) {
        config = rhs.config;
        _pollFds = rhs._pollFds;
        _serverSocketFds = rhs._serverSocketFds;
    }
    return *this;
}

Service::Service(const Config &config)
    :config(config) {
    _pollFds.resize(config.getPorts().size());
}

void Service::Start() {
    setupSockets();
    eventLoop();
}

void Service::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl F_GETFL failed with error: " << strerror(errno) << std::endl;
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL failed with error: " << strerror(errno) << std::endl;
    }
}

void Service::setupSockets() {
    std::vector<int> ports = config.getPorts();
    int index = 0;
    for (int i = 0; i < ports.size(); i++) {
        int port = ports[i];
        int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
        
        int opt = 1;
        if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << std::endl;
            close(serverSocketFd);
            continue;
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;


        int result = bind(serverSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (result == -1)
        {
            std::cerr << "Bind failed with error: " << strerror(errno) << std::endl;
            close(serverSocketFd);
            continue;
        }
        result = listen(serverSocketFd, _backLog);
        if (result == -1)
        {
            std::cerr << "Listen failed with error: " << strerror(errno) << std::endl;
            close(serverSocketFd);
            continue;
        }

        setNonBlocking(serverSocketFd);
        _pollFds[index].fd = serverSocketFd;
        _pollFds[index].events = POLLIN;
        _pollFds[index].revents = 0;

        _serverSocketFds.push_back(serverSocketFd);
        _serverSocketToPort[serverSocketFd] = port;
        index++;
        std::cout << "Port " << port << " is listening..." << std::endl;
    }
}


void Service::eventLoop() {
    while (true) {
        std::cout << "Polling..." << std::endl;
        int pollResult = poll(_pollFds.data(), _pollFds.size(), -1);
        if (pollResult == -1)
        {
            std::cerr << "Poll failed with error: " << strerror(errno) << std::endl;
            continue;
        }
        for (int i = 0; i < _pollFds.size(); i++) {
            if (_pollFds[i].revents & POLLIN) {
                if (std::find(_serverSocketFds.begin(), _serverSocketFds.end(), _pollFds[i].fd) != _serverSocketFds.end()) {  // 서버 소켓인 경우
                    int clientSocketFd = accept(_pollFds[i].fd, nullptr, nullptr);
                    setNonBlocking(clientSocketFd);
                    pollfd clientPollFd;
                    clientPollFd.fd = clientSocketFd;
                    clientPollFd.events = POLLIN;
                    clientPollFd.revents = 0;
                    _pollFds.push_back(clientPollFd);

                    // 클라이언트 소켓과 포트 번호 매핑 추가
                    _clientSocketToPort[clientSocketFd] = _serverSocketToPort[_pollFds[i].fd];
                    _bufferTable[clientSocketFd] = "";
                }
                else {  // 클라이언트 소켓인 경우
                    handleEvent(_pollFds[i].fd);
					if (_bufferTable.find(_pollFds[i].fd) == _bufferTable.end()) {
	                    _pollFds.erase(_pollFds.begin() + i);
	                    --i;
					}
                }
            }
        }
    }
}

// request body가 모두 받아졌는지 확인
bool isRequestBodyComplete(const std::string& buffer) {
    size_t pos = buffer.find("\r\n\r\n");
    // 헤더가 모두 받아졌는지 확인
    if (pos == std::string::npos)
        return false;
    std::string header = buffer.substr(0, pos);
    size_t contentLengthPos = header.find("Content-Length: ");
    if (contentLengthPos == std::string::npos)
        return true;
    size_t endOfContentLength = header.find("\r\n", contentLengthPos);
    std::string contentLength = header.substr(contentLengthPos + 16, endOfContentLength - contentLengthPos - 16);
    int length = std::stoi(contentLength);
    return buffer.size() >= pos + 4 + length;
}

size_t extractContentLength(const std::string& header)
{
    size_t contentLengthPos = header.find("Content-Length: ");
    if (contentLengthPos == std::string::npos)
        return 0;
    size_t endOfContentLength = header.find("\r\n", contentLengthPos);
    std::string contentLength = header.substr(contentLengthPos + 16, endOfContentLength - contentLengthPos - 16);
    return std::stoul(contentLength);
}

std::string extractHost(const std::string& header)
{
    size_t hostStartPos = header.find("Host: ") + 6;
    size_t hostEndPos = header.find("\r\n", hostStartPos);
    return header.substr(hostStartPos, hostEndPos - hostStartPos);
}

void Service::handleEvent(int clientSocketFd) {
	char buffer[BUFFER_SIZE];
    int size = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);    // 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
    if (size <= 0)
    {
        if (size == 0)
            std::cout << "client disconnected\n";
        else
            std::cerr << "recv failed\n";
        close(clientSocketFd);
        _bufferTable.erase(clientSocketFd);
        _clientSocketToPort.erase(clientSocketFd); // 매핑 제거
        return ;
    }
    // buffer에 읽은 내용 추가
	_bufferTable[clientSocketFd] += std::string(buffer, size);  // 바이너리 파일의 경우 null 문자가 있을 수 있으므로 size만큼만 추가

    // 헤더가 모두 받아졌는지 확인
    size_t headerEndPos = _bufferTable[clientSocketFd].find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
        return ;

    // 포트 번호 추출
    int port = _clientSocketToPort[clientSocketFd];
    // 헤더에서 host 추출
    std::string host = extractHost(_bufferTable[clientSocketFd]);
    Server server = config.selectServer(host, port);

    std::cout << "Selected server name: " << server.serverName << std::endl;

    // 헤더에서 content-length 추출
    size_t contentLength = extractContentLength(_bufferTable[clientSocketFd]);
    size_t totalLength = headerEndPos + 4 + contentLength;

    // clientMaxBodySize가 0이 아닌 경우, body size 체크, 초과시 413
    if (server.clientMaxBodySize != 0 && contentLength > server.clientMaxBodySize)
    {
        // 413 error
        HttpRequest httpRequest;
        httpRequest.parse(_bufferTable[clientSocketFd].substr(0, headerEndPos));
        HttpResponse httpResponse(server.root + "/" + server.errorPage, httpRequest, 413);
        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        // body가 큰 경우 client를 차단
        close(clientSocketFd);
        _bufferTable.erase(clientSocketFd);
        _clientSocketToPort.erase(clientSocketFd);
        return ;
    }

    // 헤더와 바디가 모두 받아졌는지 확인
    if (totalLength > _bufferTable[clientSocketFd].size())
        return ;

    std::cout << std::endl << "========== Request ==========" << std::endl << std::endl;
    std::cout << _bufferTable[clientSocketFd].substr(0, totalLength);
    std::cout << std::endl << "=============================" << std::endl << std::endl;

    // 리퀘스트 하나를 분리하여 파싱
    HttpRequest httpRequest;
    httpRequest.parse(_bufferTable[clientSocketFd].substr(0, totalLength));
    // buffer에서 처리한 리퀘스트 제거
    if (totalLength == _bufferTable[clientSocketFd].size())
        _bufferTable.erase(clientSocketFd);
    else // 다음 리퀘스트를 위해 buffer 재설정
        _bufferTable[clientSocketFd] = _bufferTable[clientSocketFd].substr(totalLength);

    int statusCode = 200;

    Location location = config.findLocation(server, httpRequest.target);

    std::cout << "Selected location path: " << location.path << std::endl;

    // method가 허용되지 않으면 405
    std::cout << "Accepted HTTP Methods: " << location.acceptedHttpMethods << std::endl;
    std::cout << "Request Method: " << httpRequest.method << std::endl;
    if (!(location.acceptedHttpMethods & httpRequest.method)) {
        statusCode = 405;
    }

    std::string uri = server.root + httpRequest.target;
    
    // 쿼리문자열 분리
    std::string queryString = "";
    size_t pos = uri.find("?");
    if (pos != std::string::npos) {
        queryString = uri.substr(pos + 1);
        uri = uri.substr(0, pos);
    }

    // uri가 존재하는지 확인
    if (access(uri.substr(1).c_str(), F_OK) == -1) {
        statusCode = 404;
    }

    if (statusCode != 200)
    {
        uri = server.root + "/" + server.errorPage;
        HttpResponse httpResponse(uri, httpRequest, statusCode);
        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
        std::cout << httpResponse.response;
        std::cout << std::endl << "=============================" << std::endl << std::endl;
        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        close(clientSocketFd);
        _clientSocketToPort.erase(clientSocketFd);
        return ;
    }
    if (httpRequest.method == GET)
        getMethod(uri, httpRequest, location, statusCode, clientSocketFd, server, queryString);
    else if (httpRequest.method == POST)
        postMethod(uri, httpRequest, server, location, clientSocketFd);
    else if (httpRequest.method == DELETE)
        deleteMethod(uri, httpRequest, statusCode, clientSocketFd);
    else
        statusCode = 405;
    close(clientSocketFd);
    _clientSocketToPort.erase(clientSocketFd);
}

void Service::getMethod(std::string& uri,
                        HttpRequest& httpRequest,
                        const Location& location,
                        int& statusCode,
                        int& clientSocketFd,
                        Server& server,
                        std::string& queryString) {
    /*
        1. uri가 디렉토리인지 확인
        1-1. 디렉토리인 경우
            uri가 /로 끝나지 않는 경우 403
            index 존재
            autoindex가 true인 경우
        1-2. 디렉토리가 아닌 경우
    */
    if (isDirectory(uri.substr(1))) {
        if (uri.back() != '/')
            statusCode = 404;
        else {
            if (!location.index.empty())
            {
                if (access((uri.substr(1) + location.index).c_str(), F_OK) == 0)
                    uri += location.index;
                else
                    statusCode = 404;
            } else if (!location.autoIndex) {
                statusCode = 403;
            }
        }
    } else {
        if (uri.back() == '/')
            statusCode = 404;
    }

    if (statusCode != 200)
    {
        uri = server.root + "/" + server.errorPage;
        HttpResponse httpResponse(uri, httpRequest, statusCode);
        std::cout << std::endl
                  << "========== Response =========" << std::endl
                  << std::endl;
        std::cout << httpResponse.response;
        std::cout << std::endl
                  << "=============================" << std::endl
                  << std::endl;
        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        return;
    }

    HttpResponse httpResponse(uri, httpRequest, statusCode, queryString);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.response;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

void parse_multipart_data(const std::string& path, const std::string& body, const std::string& boundary) {
    std::vector<std::string> parts = split(body, boundary);
    for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
        std::string part = *it;
        if (part.empty() || part == "--\r\n" || part == "--") {
            continue;
        }

        // 헤더와 바디 추출
        size_t header_end_pos = part.find("\r\n\r\n");
        if (header_end_pos == std::string::npos) {
            continue;
        }
        std::string headers = part.substr(0, header_end_pos);
        std::string body = part.substr(header_end_pos + 4);

        // 파일이름 추출
        size_t cd_pos = headers.find("Content-Disposition:");
        if (cd_pos == std::string::npos) {
            continue;
        }

        std::string content_disposition = headers.substr(cd_pos, headers.find("\r\n", cd_pos) - cd_pos);
        size_t filename_pos = content_disposition.find("filename=");
        if (filename_pos != std::string::npos) {
            std::string filename = content_disposition.substr(filename_pos + 10);
            filename = filename.substr(0, filename.find("\""));

            // Save file
            std::ofstream outfile(path + filename, std::ios::binary);
            outfile.write(body.data(), body.size());
            outfile.close();

            std::cout << "Saved file: " << filename << std::endl;
        }
    }
}

void Service::postMethod(std::string& uri, HttpRequest& httpRequest, const Server& server, const Location& location, int& clientSocketFd) {
    int statusCode = 201;
    // POST 요청 처리
    // uri가 디렉토리인지 확인
    if (!isDirectory(uri.substr(1)))
        statusCode = 404;
    std::string contentType = httpRequest.headers["Content-Type"];
    std::cout << contentType << std::endl;
    if (contentType.find("multipart/form-data") != std::string::npos) {
        std::string boundary = "--" + contentType.substr(contentType.find("boundary=") + 9);
        std::string path = server.root + location.path;
        if (isDirectory(path.substr(1)))
        {
            std::cout << "Multipart data received" << std::endl;
            if (path.back() != '/')
                path += '/';
            parse_multipart_data(path.substr(1), httpRequest.body, boundary);
        }
        else
            statusCode = 404;
    }
    else
        statusCode = 400;
    HttpResponse httpResponse(uri, httpRequest, statusCode);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.response;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
}

void Service::deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd) {
    // DELETE 요청 처리
    // uri가 존재하는지 확인
    if (access(uri.substr(1).c_str(), F_OK) == -1)
        statusCode = 404;
    else
        remove(uri.substr(1).c_str());
    HttpResponse httpResponse(uri, httpRequest, statusCode);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.response;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
}
