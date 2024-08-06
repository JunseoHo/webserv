#include "Core.hpp"

Core::Core() { /* DO NOTHING */ }
Core::Core(const Core& obj) { /* THIS IS PRIVATE */ }
Core::~Core() { /* DO NOTHING */ }

Core::Core(const Config &config)
    :config(config) {
    _pollFds.resize(config.GetAllListeningPorts().size());
}

Core& Core::operator= (const Core& rhs) { /* THIS IS PRIVATE */ }

void Core::Start() {
	_socketManager = SocketManager(config.GetAllListeningPorts());
	
	std::map<int, int> socketFdPortMap = _socketManager.GetSocketFdPortMap();
	int i = 0;
	for (std::map<int, int>::iterator it = socketFdPortMap.begin(); it != socketFdPortMap.end(); ++it, ++i) {
        _pollFds[i].fd = it->first;
        _pollFds[i].events = POLLIN;
        _pollFds[i].revents = 0;
		std::cout << "File descriptor " << it->first << " is polling..." << std::endl;
    }

    eventLoop();
}


void Core::eventLoop() {
    while (true) {
        try {
            // std::cout << "Polling..." << std::endl;
            int pollResult = poll(_pollFds.data(), _pollFds.size(), -1);
            if (pollResult == -1)
                throw std::runtime_error("poll failed");
            for (int i = 0; i < _pollFds.size(); i++) {
                if (_pollFds[i].revents & POLLIN) {
                    if (_socketManager.isServerSocketFd(_pollFds[i].fd)) {  // 서버 소켓인 경우
                        pollfd clientPollFd = _socketManager.newClientPollfd(_pollFds[i].fd);
                        _pollFds.push_back(clientPollFd);
                        _bufferManager.addBuffer(clientPollFd.fd);
                    } else if (_socketManager.isConnectedCgiToClient(_pollFds[i].fd)) { // CGI PIPE인 경우
                        handleCgiEvent(_pollFds[i].fd, _socketManager.GetClientFdByCgiFd(_pollFds[i].fd));
                        // if (_cgiBufferManager.isBufferEmpty(_pollFds[i].fd)
                        //     && !_socketManager.isConnectedCgiToClient(_pollFds[i].fd)) {
                        //     _pollFds.erase(_pollFds.begin() + i);
                        //     --i;
                        // }
                    } else if (!_socketManager.isConnectedClinetToCgi(_pollFds[i].fd)) {  // 클라이언트 소켓인 경우
                        handleEvent(_pollFds[i].fd);
                        // if (_bufferManager.isBufferEmpty(_pollFds[i].fd)
                        //     && !_socketManager.isConnectedClinetToCgi(_pollFds[i].fd)) {
                        //     _pollFds.erase(_pollFds.begin() + i);
                        //     --i;
                        // }
                    }
                }
                if (_pollFds[i].revents & POLLOUT) {
                    handleOutEvent(_pollFds[i].fd);
                    // if (_responseBufferManager.isBufferEmpty(_pollFds[i].fd)) {
                    //     _pollFds.erase(_pollFds.begin() + i);
                    //     --i;
                    // }
                }
            }
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void Core::handleOutEvent(int clientSocketFd) {
    if (_responseBufferManager.isBufferEmpty(clientSocketFd))
        return ;
    std::string response = _responseBufferManager.GetSubBuffer(clientSocketFd, BUFFER_SIZE);
    std::cout << response << std::endl;
    ssize_t size = write(clientSocketFd, response.c_str(), response.size());
    if (size < 0)
    {
        close(clientSocketFd);
        _responseBufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);
        return ;
    }
    else if (size == 0)
    {
        close(clientSocketFd);
        _responseBufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);   
        return ;
    }
    if (size != response.size())
    {
        close(clientSocketFd);
        _responseBufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);
        return ;
    }

    _responseBufferManager.removeBufferFront(clientSocketFd, size);

    if (_responseBufferManager.isBufferEmpty(clientSocketFd))
    {
        close(clientSocketFd);
        _responseBufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);
    }
}

void Core::handleEvent(int clientSocketFd) {
	char buffer[BUFFER_SIZE];
    ssize_t size = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);    // 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
    if (size <= 0)
    {
        if (size == 0)
            std::cout << "client disconnected\n";
        else
            std::cerr << "recv failed:" << strerror(errno) << "\n";
        close(clientSocketFd);
        _bufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);
        return ;
    }
    // buffer에 읽은 내용 추가
	_bufferManager.appendBuffer(clientSocketFd, buffer, size);

    // 헤더가 모두 받아졌는지 확인
    size_t headerEndPos = _bufferManager.GetBuffer(clientSocketFd).find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
        return ;

    // 포트 번호 추출
    int port = _socketManager.GetPortBySocketFd(clientSocketFd);
    // 헤더에서 host 추출
    std::string host = getHeaderValue(_bufferManager.GetBuffer(clientSocketFd), "Host");
    Server server = config.SelectProcessingServer(host, port);

    std::cout << "Selected server name: " << server.serverName << std::endl;

    // 헤더에서 content-length 추출
    std::string contentLengthString = getHeaderValue(_bufferManager.GetBuffer(clientSocketFd), "Content-Length");
    size_t contentLength = contentLengthString.empty() ? 0 : std::stoi(contentLengthString);
    size_t totalLength = headerEndPos + 4 + contentLength;

    // clientMaxBodySize가 0이 아닌 경우, body size 체크, 초과시 413
    if (server.clientMaxBodySize != 0 && contentLength > server.clientMaxBodySize)
    {
        // 413 error
        HttpRequest httpRequest;
        httpRequest.parse(_bufferManager.GetBuffer(clientSocketFd).substr(0, headerEndPos));
        HttpResponse httpResponse(server.root + "/" + server.errorPage, httpRequest, 413);
        send(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
        // body가 큰 경우 client를 차단
        close(clientSocketFd);
        _bufferManager.removeBuffer(clientSocketFd);
        _socketManager.removeClientSocketFd(clientSocketFd);
        return ;
    }

    // 헤더와 바디가 모두 받아졌는지 확인
    if (totalLength > _bufferManager.GetBuffer(clientSocketFd).size())
        return ;

    std::cout << std::endl << "========== Request ==========" << std::endl << std::endl;
    std::cout << _bufferManager.GetBuffer(clientSocketFd).substr(0, totalLength);
    std::cout << std::endl << "=============================" << std::endl << std::endl;

    // 리퀘스트 하나를 분리하여 파싱
    HttpRequest httpRequest;
    httpRequest.parse(_bufferManager.GetBuffer(clientSocketFd).substr(0, totalLength));
    // buffer에서 처리한 리퀘스트 제거
    if (totalLength == _bufferManager.GetBuffer(clientSocketFd).size())
        _bufferManager.removeBuffer(clientSocketFd);
    else // 다음 리퀘스트를 위해 buffer 재설정
        _bufferManager.removeBufferFront(clientSocketFd, totalLength);

    int statusCode = 200;

    Location location = config.FindOptimalLocation(server, httpRequest.target);

    std::cout << "Selected location path: " << location.path << std::endl;

    // method가 허용되지 않으면 405
    std::cout << "Accepted HTTP Methods: " << location.acceptedHttpMethods << std::endl;
    std::cout << "Request Method: " << httpRequest.method << std::endl;
    if (!(location.acceptedHttpMethods & httpRequest.method)) {
        statusCode = 405;
    }

    std::string uri = server.root + httpRequest.target;
    std::string target = uri.find("?") != std::string::npos ? uri.substr(0, uri.find("?")) : uri;

    if (access(target.substr(1).c_str(), F_OK) == -1 && uri.find("/cgi-bin/") == std::string::npos)
        statusCode = 404;

    if (statusCode != 200)
    {
        std::cerr << "Status code: " << statusCode << std::endl;
        uri = server.root + "/" + server.errorPage;
        HttpResponse httpResponse(uri, httpRequest, statusCode);
        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
        std::cout << httpResponse.full;
        std::cout << std::endl << "=============================" << std::endl << std::endl;
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        // send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        // close(clientSocketFd);
        // _socketManager.removeClientSocketFd(clientSocketFd);
        return ;
    }
    if (uri.find("/cgi-bin/") != std::string::npos)
    {
        executeCGI(uri.substr(1), httpRequest, clientSocketFd, location);
        return ;
    }
    else if (httpRequest.method == GET)
        getMethod(uri, httpRequest, location, statusCode, clientSocketFd, server);
    else if (httpRequest.method == POST)
        postMethod(uri, httpRequest, server, location, clientSocketFd);
    else if (httpRequest.method == DELETE)
        deleteMethod(uri, httpRequest, statusCode, clientSocketFd);
    else
        statusCode = 405;
    // close(clientSocketFd);
    // _socketManager.removeClientSocketFd(clientSocketFd);
}

void Core::executeCGI(const std::string& uri, HttpRequest &request, int clientSocketFd, Location location)
{
    int cgiInPipe[2];
    int cgiOutPipe[2];
    if (pipe(cgiInPipe) == -1)
    {
        std::cerr << "Failed to create pipe: " << std::strerror(errno) << '\n';
        return;
    }
    if (pipe(cgiOutPipe) == -1)
    {
        std::cerr << "Failed to create pipe: " << std::strerror(errno) << '\n';
        return;
    }
    setNonBlocking(cgiInPipe[0]);
    setNonBlocking(cgiInPipe[1]);
    setNonBlocking(cgiOutPipe[0]);
    setNonBlocking(cgiOutPipe[1]);

    pid_t pid = fork();
    if (pid == -1)
    {
        std::cerr << "Failed to fork: " << std::strerror(errno) << '\n';
        return;
    }

    if (pid == 0)
    {
        close(cgiInPipe[1]);
        close(cgiOutPipe[0]);
        dup2(cgiInPipe[0], STDIN_FILENO);
        dup2(cgiOutPipe[1], STDOUT_FILENO);
        close(cgiInPipe[0]);
        close(cgiOutPipe[1]);

        std::string scriptPath = uri.substr(0, uri.find(".py") + 3);
        std::string pathInfo = uri.substr(uri.find(".py") + 3, uri.find("?") - uri.find(".py") - 3);
        std::string queryString = uri.substr(uri.find("?") + 1);

        std::string interpreter = location.cgiPath;
        std::string script = uri.substr(uri.find("cgi-bin/") + 8);
        if (script.empty())
            script = "time.py";
        char **args = (char **)malloc(sizeof(char *) * 3);
        args[0] = strdup(interpreter.c_str());
        args[1] = strdup(("cgi-bin/" + script).c_str());
        args[2] = NULL;
        for (int i = 0; args[i] != NULL; i++)
            std::cerr << "args[" << i << "]: " << args[i] << std::endl;
        std::vector<std::string> envStrings;
        envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
        envStrings.push_back("PATH_INFO=" + pathInfo);
        envStrings.push_back("QUERY_STRING=" + queryString);
        std::string method = request.method == GET ? "GET" : "POST";
        envStrings.push_back("REQUEST_METHOD=" + method);
        envStrings.push_back("REDIRECT_STATUS=200");
        envStrings.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));
        envStrings.push_back("CONTENT_TYPE=" + request.headers["Content-Type"]);
        envStrings.push_back("SERVER_PROTOCOL=" + request.version);
        envStrings.push_back("SERVER_SOFTWARE=webserv");
        envStrings.push_back("SERVER_NAME=" + request.headers["Host"]);
        envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");

        // char* 배열로 변환
        std::vector<char*> env(envStrings.size() + 1); // +1 for NULL terminator
        for (size_t i = 0; i < envStrings.size(); ++i) {
            env[i] = const_cast<char*>(envStrings[i].c_str());
        }
        env[envStrings.size()] = NULL; // NULL terminator

        execve(args[0], args, env.data());
        exit(1);
    }
    close(cgiInPipe[0]);
    close(cgiOutPipe[1]);
    // request 전문을 파이프로 전달
    if (request.method == POST) {
        pollfd pollFdOut;
        pollFdOut.fd = cgiInPipe[1];
        pollFdOut.events = POLLOUT;
        pollFdOut.revents = 0;
        _pollFds.push_back(pollFdOut);
        _responseBufferManager.appendBuffer(cgiInPipe[1], request.full.c_str(), request.full.size());
    }
    pollfd pollFd;
    pollFd.fd = cgiOutPipe[0];
    pollFd.events = POLLIN;
    pollFd.revents = 0;
    _pollFds.push_back(pollFd);
    _cgiBufferManager.addBuffer(cgiOutPipe[0]);
    _socketManager.connectCgiToClient(cgiOutPipe[0], clientSocketFd);
}

void Core::handleCgiEvent(int cgiPipeFd, int clientFd)
{
    char buffer[BUFFER_SIZE];
    ssize_t size = read(cgiPipeFd, buffer, BUFFER_SIZE - 1);
    if (size > 0) {
		_cgiBufferManager.appendBuffer(cgiPipeFd, buffer, size);
        return;
    }

	if (size < 0)
	{
		std::cerr << "read failed: " << strerror(errno) << '\n';
		HttpResponse httpResponse("", HttpRequest(), 500);
		send(clientFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
	}
	else
	{
        HttpResponse response(_cgiBufferManager.GetBuffer(cgiPipeFd));
        std::cout << response.full.c_str() << std::endl;
        _responseBufferManager.appendBuffer(clientFd, response.full.c_str(), response.full.size());
        // send(clientFd, response.full.c_str(), response.full.size(), 0);
	}
	close(cgiPipeFd);
    close(clientFd);
    _socketManager.removeClientSocketFd(clientFd);
    _cgiBufferManager.removeBuffer(cgiPipeFd);
    _socketManager.disconnectCgiToClient(cgiPipeFd);

}

void Core::getMethod(std::string& uri,
                        HttpRequest& httpRequest,
                        const Location& location,
                        int& statusCode,
                        int& clientSocketFd,
                        Server& server) {
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
        std::cout << httpResponse.full;
        std::cout << std::endl
                  << "=============================" << std::endl
                  << std::endl;
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        // send(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
        return;
    }

    HttpResponse httpResponse(uri, httpRequest, statusCode);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.full;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
    // send(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
}

void Core::postMethod(std::string& uri, HttpRequest& httpRequest, const Server& server, const Location& location, int& clientSocketFd) {
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
    std::cout << httpResponse.full;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
    // send(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
}

void Core::deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd) {
    // DELETE 요청 처리
    // uri가 존재하는지 확인
    if (access(uri.substr(1).c_str(), F_OK) == -1)
        statusCode = 404;
    else
        remove(uri.substr(1).c_str());
    HttpResponse httpResponse(uri, httpRequest, statusCode);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.full;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
    // send(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size(), 0);
}
