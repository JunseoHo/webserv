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
        _resourcesPath = rhs._resourcesPath;
        config = rhs.config;
        _pollFds = rhs._pollFds;
        _serverSocketFds = rhs._serverSocketFds;
    }
    return *this;
}

Service::Service(const Config &config, const std::string& root)
    : _resourcesPath(root), config(config) {
    _pollFds.resize(config.getPorts().size());
}

void Service::Start() {
    setupSockets();
    eventLoop();
}

void Service::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Service::setupSockets() {
    std::vector<int> ports = config.getPorts();
    int index = 0;
    for (int i = 0; i < ports.size(); i++) {
        int port = ports[i];
        int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        bind(serverSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        listen(serverSocketFd, _backLog);

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
        poll(_pollFds.data(), _pollFds.size(), -1); 
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
                    if (handleEvent(_pollFds[i].fd))
					{
                        _bufferTable.erase(_pollFds[i].fd);
                        _clientSocketToPort.erase(_pollFds[i].fd); // 매핑 제거
                    	close(_pollFds[i].fd);
	                    _pollFds.erase(_pollFds.begin() + i);
	                    --i;
					}
                }
            }
        }
    }
}

bool isDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR)
            return true;
    }
    return false;
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

bool Service::handleEvent(int clientSocketFd) {
	char buffer[BUFFER_SIZE];
    int size = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);    // 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
	_bufferTable[clientSocketFd] += std::string(buffer, size);  // 바이너리 파일의 경우 null 문자가 있을 수 있으므로 size만큼만 추가
    // 헤더가 모두 받아졌는지 확인

	if (isRequestBodyComplete(_bufferTable[clientSocketFd]))    // 리퀘스트의 헤더와 바디가 모두 받아졌는지 확인
	{
		std::cout << std::endl << "========== Request ==========" << std::endl << std::endl;
	    std::cout << _bufferTable[clientSocketFd];
	    std::cout << std::endl << "=============================" << std::endl << std::endl;

	    HttpRequest httpRequest;
	    httpRequest.parse(_bufferTable[clientSocketFd]);

	    // 요청이 들어온 포트 번호를 가져옴
	    int port = _clientSocketToPort[clientSocketFd];
	    Server server = config.selectServer(httpRequest, port);

		std::cout << "Selected server name: " << server.serverName << std::endl;

        int statusCode = 200;

	    // server clientMaxBodySize 가 0이 아닌 경우, body size 체크, 초과시 413
	    if (server.clientMaxBodySize != 0 && httpRequest.body.size() > server.clientMaxBodySize) {
            statusCode = 413;
	    }

	    Location location = config.findLocation(server, httpRequest.target);

		std::cout << "Selected location path: " << location.path << std::endl;

	    // method가 허용되지 않으면 405
        std::cout << "Accepted HTTP Methods: " << location.acceptedHttpMethods << std::endl;
        std::cout << "Request Method: " << httpRequest.method << std::endl;
	    if (!(location.acceptedHttpMethods & httpRequest.method)) {
            statusCode = 405;
	    }
        std::cout << "server.root: " << server.root << std::endl;
        std::cout << "httpRequest.referer: " << httpRequest.referer << std::endl;
		std::string uri = server.root + httpRequest.referer;
		// std::cout << "URI: " << uri << std::endl;

        uri += httpRequest.target;
        // std::cout << uri.length() << std::endl;
		// std::cout << "URI: " << uri.c_str() << std::endl;
        // std::cout << "server.root: " << server.root << server.root.length() << std::endl;
        // std::cout << "httpRequest.referer: " << httpRequest.referer << httpRequest.referer.length() << std::endl;
        // std::cout << "httpRequest.target: " << httpRequest.target << httpRequest.target.length() << std::endl;

        // std::cout << "server.root: " << server.root << std::endl;
        // std::cout << "httpRequest.referer: " << httpRequest.referer << std::endl;
        // std::cout << "httpRequest.target: " << httpRequest.target << std::endl;
        // std::cout << "result: " << server.root + httpRequest.referer + httpRequest.target << std::endl;
        
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
            return true;
        }
        if (httpRequest.method == GET)
            getMethod(uri, httpRequest, location, statusCode, clientSocketFd);
        else if (httpRequest.method == POST)
            postMethod(uri, httpRequest, server, location, clientSocketFd);
        else if (httpRequest.method == DELETE)
            deleteMethod(uri, httpRequest, statusCode, clientSocketFd);
        else
            statusCode = 405;
		return true;
	}
	return false;
}

void Service::getMethod(std::string& uri,
                        HttpRequest& httpRequest,
                        const Location& location,
                        int& statusCode,
                        int& clientSocketFd) {
    // uri가 디렉토리인 경우 index 파일로 리다이렉트
    if (isDirectory(uri.substr(1))) {
        if (uri.back() != '/')
            uri += '/';
        uri += location.index;
    }
    HttpResponse httpResponse(uri, httpRequest, statusCode);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.response;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
}

void Service::postMethod(std::string& uri, HttpRequest& httpRequest, const Server& server, const Location& location, int& clientSocketFd) {
    int statusCode = 201;
    // POST 요청 처리
    // uri가 디렉토리인지 확인
    if (!isDirectory(uri.substr(1)))
        statusCode = 404;
    std::string contentType = httpRequest.headers["Content-Type"];
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos != std::string::npos) {
        std::string boundary = contentType.substr(boundaryPos + 9);
        std::string boundaryEnd = "--" + boundary + "--";
        size_t pos = _bufferTable[clientSocketFd].find(boundaryEnd);
        if (pos == std::string::npos)
            statusCode = 400;
        else {
            std::string body = _bufferTable[clientSocketFd].substr(_bufferTable[clientSocketFd].find(boundary) + boundary.size() + 2, pos - _bufferTable[clientSocketFd].find(boundary) - boundary.size() - 2);
            std::string path = server.root + location.path;
            if (isDirectory(path.substr(1))) {
                if (path.back() != '/')
                    path += '/';
                std::string filename = path + body.substr(body.find("filename=\"") + 10, body.find("\"", body.find("filename=\"") + 10) - body.find("filename=\"") - 10);
                std::ofstream file(filename.substr(1), std::ios::binary);
                // body에서 boundary 데이터만 추출
                body = body.substr(body.find(boundary) + boundary.size() + 2);
                // body에서 바이너리 데이터만 추출
                body = body.substr(body.find("\r\n\r\n") + 4);
                // body에서 마지막 엔터 제거
                body = body.substr(0, body.size() - 2);
                std::cout << "---------- File Content ----------" << std::endl;
                std::cout << body << std::endl;
                std::cout << "----------------------------------" << std::endl;
                file.write(body.c_str(), body.size());
                file.close();
            }
            else
                statusCode = 404;
        }
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
