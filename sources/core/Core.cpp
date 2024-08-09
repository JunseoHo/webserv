#include "Core.hpp"

Core::Core() { /* DO NOTHING */ }
Core::Core(const Core& obj) { /* THIS IS PRIVATE */ }
Core::~Core() { /* DO NOTHING */ }

Core::Core(const Config &config)
    :config(config) {
    _pollFds.resize(config.GetAllListeningPorts().size());
}

Core& Core::operator= (const Core& rhs) { /* THIS IS PRIVATE */ return *this; }

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
                        int clientFd = _socketManager.GetClientFdByCgiFd(_pollFds[i].fd);
                        handleCgiEvent(_pollFds[i].fd, clientFd);
                    } else if (!_socketManager.isConnectedClinetToCgi(_pollFds[i].fd)) {  // 클라이언트 소켓인 경우
                        handleEvent(_pollFds[i].fd);
                        if (!_responseBufferManager.isBufferEmpty(_pollFds[i].fd))
                            _pollFds[i].events = POLLOUT;
                    }
                } else if (_pollFds[i].revents & POLLOUT) {
                    handleOutEvent(_pollFds[i].fd);
                    if (_responseBufferManager.isBufferEmpty(_pollFds[i].fd))
                    {
                        if (_bufferManager.isBufferEmpty(_pollFds[i].fd) && _cgiBufferManager.isBufferEmpty(_pollFds[i].fd))
                        {
                            _pollFds.erase(_pollFds.begin() + i);
                            i--;
                        }
                        else
                            _pollFds[i].events = POLLIN;
                    }
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
    //std::cout << "========== Response =========" << std::endl;
    //std::cout << response << '\n';
    //std::cout << "=============================" << std::endl;
    ssize_t size = write(clientSocketFd, response.c_str(), response.size());;
    if (size < 0)
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
    ssize_t size = read(clientSocketFd, buffer, BUFFER_SIZE - 1);    // 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
    if (size <= 0)
    {
        for (int i = 0; i < _pollFds.size(); i++)
        {
            if (_pollFds[i].fd == clientSocketFd)
            {
                _pollFds.erase(_pollFds.begin() + i);
                break;
            }
        }
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

    // method가 허용되지 않으면 
    std::cout << "Accepted HTTP Methods: " << location.acceptedHttpMethods << std::endl;
    std::cout << "Request Method: " << httpRequest.method << std::endl;
    if (!(location.acceptedHttpMethods & httpRequest.method)) {
        statusCode = 405;
    }

    std::string uri = location.root + httpRequest.target;
    std::string target = uri.find("?") != std::string::npos ? uri.substr(0, uri.find("?")) : uri;

    if (access(target.substr(1).c_str(), F_OK) == -1 && (uri.find("/cgi-bin/") == std::string::npos || httpRequest.target.find("/cgi-bin/") != 0))
	{
		if (httpRequest.method != POST && statusCode == 200)
        	statusCode = 404;
	}

    if (statusCode != 200)
    {
        uri = location.root + "/" + location.errorPage;
        HttpResponse httpResponse(uri, httpRequest, statusCode);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return ;
    }
    if (uri.find("/cgi-bin/") != std::string::npos)
        executeCGI(uri.substr(1), httpRequest, clientSocketFd, location);
    else if (httpRequest.method == GET)
        getMethod(uri, httpRequest, location, statusCode, clientSocketFd);
    else if (httpRequest.method == POST)
        postMethod(uri, httpRequest, location, clientSocketFd);
    else if (httpRequest.method == DELETE)
        deleteMethod(uri, httpRequest, statusCode, clientSocketFd);
    else
        statusCode = 405;
}

void Core::executeCGI(std::string uri, HttpRequest &request, int clientSocketFd, Location location)
{
    std::string scriptPath = uri.substr(0, uri.find(".py") + 3);
    std::string pathInfo = uri.substr(uri.find(".py") + 3, uri.find("?") - uri.find(".py") - 3);
    std::string queryString = uri.substr(uri.find("?") + 1);

    std::string interpreter = location.cgiPath;
    std::string script = uri.substr(uri.find("cgi-bin/") + 8);

    std::cerr << "scriptPath: " << scriptPath << '\n';
    std::cerr << "pathInfo: " << pathInfo << '\n';
    std::cerr << "queryString: " << queryString << '\n';
    std::cerr << "interpreter: " << interpreter << '\n';
    std::cerr << "script: " << script << '\n';
    std::cerr << "uri: " << uri << '\n';

    if (script.empty() && !location.index.empty())
        script = location.index;
    else if (script.empty() && location.index.empty())
    {
        HttpResponse httpResponse(location.root + "/" + location.errorPage, request, 404);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return;
    }

    if (access(("cgi-bin/" + script).c_str(), F_OK) == -1 || script.find(".py") == std::string::npos)
    {
        HttpResponse httpResponse(location.root + "/" + location.errorPage, request, 404);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return;
    }

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

        
        char **args = (char **)malloc(sizeof(char *) * 3);
        args[0] = strdup(interpreter.c_str());
        args[1] = strdup(("cgi-bin/" + script).c_str());
        args[2] = NULL;
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
        _responseBufferManager.appendBuffer(cgiInPipe[1], request.body.c_str(), request.body.size());
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
		_responseBufferManager.appendBuffer(clientFd, httpResponse.full.c_str(), httpResponse.full.size());
	}
	else
	{
        HttpResponse response(_cgiBufferManager.GetBuffer(cgiPipeFd));
        _responseBufferManager.appendBuffer(clientFd, response.full.c_str(), response.full.size());
	}
    for (int i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == clientFd)
        {
            _pollFds[i].events = POLLOUT;
            break;
        }
    }
	close(cgiPipeFd);
    _cgiBufferManager.removeBuffer(cgiPipeFd);
    _socketManager.disconnectCgiToClient(cgiPipeFd);
}

void Core::getMethod(std::string& uri,
                        HttpRequest& httpRequest,
                        const Location& location,
                        int& statusCode,
                        int& clientSocketFd) {
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
        uri = location.root + "/" + location.errorPage;
        HttpResponse httpResponse(uri, httpRequest, statusCode);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return;
    }

    HttpResponse httpResponse(uri, httpRequest, statusCode);
    _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
}

void Core::postMethod(std::string& uri, HttpRequest& httpRequest, const Location& location, int& clientSocketFd) {
	std::string scriptPath = "resources/cgi-bin/post.py";
    std::string pathInfo = httpRequest.target;
    std::string queryString = "";
    std::string interpreter = "/usr/local/bin/python3"; // revise
    std::string script = "post.py";
    std::cerr << "scriptPath: " << scriptPath << '\n';
    std::cerr << "pathInfo: " << pathInfo << '\n';
    std::cerr << "queryString: " << queryString << '\n';
    std::cerr << "interpreter: " << interpreter << '\n';
    std::cerr << "script: " << script << '\n';
    std::cerr << "uri: " << uri << '\n';
    if (script.empty() && !location.index.empty())
        script = location.index;
    else if (script.empty() && location.index.empty())
    {
        HttpResponse httpResponse(location.root + "/" + location.errorPage, httpRequest, 404);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return;
    }
    if (access(("cgi-bin/" + script).c_str(), F_OK) == -1 || script.find(".py") == std::string::npos)
    {
        HttpResponse httpResponse(location.root + "/" + location.errorPage, httpRequest, 404);
        _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
        return;
    }
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

        
        char **args = (char **)malloc(sizeof(char *) * 3);
        args[0] = strdup(interpreter.c_str());
        args[1] = strdup(("cgi-bin/" + script).c_str());
        args[2] = NULL;
        std::vector<std::string> envStrings;
        envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
        envStrings.push_back("PATH_INFO=" + pathInfo);
        envStrings.push_back("QUERY_STRING=" + queryString);
        std::string method = httpRequest.method == GET ? "GET" : "POST";
        envStrings.push_back("REQUEST_METHOD=" + method);
        envStrings.push_back("REDIRECT_STATUS=200");
        envStrings.push_back("CONTENT_LENGTH=" + std::to_string(httpRequest.body.size()));
        envStrings.push_back("CONTENT_TYPE=" + httpRequest.headers["Content-Type"]);
        envStrings.push_back("SERVER_PROTOCOL=" + httpRequest.version);
        envStrings.push_back("SERVER_SOFTWARE=webserv");
        envStrings.push_back("SERVER_NAME=" + httpRequest.headers["Host"]);
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
    if (httpRequest.method == POST) {
        pollfd pollFdOut;
        pollFdOut.fd = cgiInPipe[1];
        pollFdOut.events = POLLOUT;
        pollFdOut.revents = 0;
        _pollFds.push_back(pollFdOut);
        _responseBufferManager.appendBuffer(cgiInPipe[1], httpRequest.body.c_str(), httpRequest.body.size());
    }
    pollfd pollFd;
    pollFd.fd = cgiOutPipe[0];
    pollFd.events = POLLIN;
    pollFd.revents = 0;
    _pollFds.push_back(pollFd);
    _cgiBufferManager.addBuffer(cgiOutPipe[0]);
    _socketManager.connectCgiToClient(cgiOutPipe[0], clientSocketFd);
}

void Core::deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd) {
    // DELETE 요청 처리
    // uri가 존재하는지 확인
    if (access(uri.substr(1).c_str(), F_OK) == -1)
        statusCode = 404;
    else
        remove(uri.substr(1).c_str());
    HttpResponse httpResponse(uri, httpRequest, statusCode);
    _responseBufferManager.appendBuffer(clientSocketFd, httpResponse.full.c_str(), httpResponse.full.size());
}
