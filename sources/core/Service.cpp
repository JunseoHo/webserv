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


bool Service::handleEvent(int clientSocketFd) {
	std::cout << "Handle!" << std::endl;
	char buffer[BUFFER_SIZE];
    int size = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);	// 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
	buffer[size] = '\0';
	_bufferTable[clientSocketFd] += buffer;
	if (size < BUFFER_SIZE - 1)
	{
		std::cout << "Write Up!" << std::endl;
		std::cout << std::endl << "========== Request ==========" << std::endl << std::endl;
	    std::cout << _bufferTable[clientSocketFd];
	    std::cout << std::endl << "=============================" << std::endl << std::endl;

	    HttpRequest httpRequest;
	    httpRequest.parse(_bufferTable[clientSocketFd]);

	    // 요청이 들어온 포트 번호를 가져옴
	    int port = _clientSocketToPort[clientSocketFd];
	    Server server = config.selectServer(httpRequest, port);

	    // server clientMaxBodySize 가 0이 아닌 경우, body size 체크, 초과시 413
	    //if (server.clientMaxBodySize != 0 && httpRequest.body.size() > server.clientMaxBodySize) {
	    //    HttpResponse httpResponse(server, httpRequest, 413);
	    //    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
	    //    std::cout << httpResponse.response;
	    //    std::cout << std::endl << "=============================" << std::endl << std::endl;
	    //    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
	    //    return true;
	    //}

        if (httpRequest.target.back() == '/')
            httpRequest.target = "/" + server.index;

	    Route route;
	    for (std::list<Route>::const_iterator it = server.routes.begin(); it != server.routes.end(); it++) {
            std::cout << server.root + it->location + httpRequest.target << std::endl;
	        if (access((server.root + it->location + httpRequest.target).c_str(), F_OK) != -1) {
                route = *it;
	            break;
	        }
	    }

	    if (route.location.empty())
	    {
	        HttpResponse httpResponse(server, httpRequest, 404);
	        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
	        std::cout << httpResponse.response;
	        std::cout << std::endl << "=============================" << std::endl << std::endl;
	        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
	        return true;
	    }

	    // method가 허용되지 않으면 405
	    if (!(route.acceptedHttpMethods & httpRequest.method)) {
	        HttpResponse httpResponse(server, httpRequest, 405);
	        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
	        std::cout << httpResponse.response;
	        std::cout << std::endl << "=============================" << std::endl << std::endl;
	        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
	        return true;
	    }

	    HttpResponse httpResponse(server, httpRequest, 200);
	    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
	    std::cout << httpResponse.response;
	    std::cout << std::endl << "=============================" << std::endl << std::endl;
	    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
		return true;
	}
	return false;
}











