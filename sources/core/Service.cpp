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

Service::Service(const Config &config, const std::string& resourcesPath)
    : _resourcesPath(resourcesPath), config(config) {
    _pollFds.resize(config.getPorts().size());
}

// std::string readFileToString(const std::string& filename) {
//     std::ifstream file(filename);
//     if (!file.is_open()) {
//         std::cerr << "Failed to open file: " << filename << std::endl;
//         return "";
//     }
//     std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     file.close();
//     return content;
// }

// bool endsWith(const std::string& str, const std::string& suffix) {
//     if (str.length() < suffix.length()) return false;
//     return str.substr(str.length() - suffix.length()) == suffix;
// }

void Service::Start() {
    setupSockets();
    eventLoop();
}

void Service::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Service::setupSockets() {
    std::list<int> ports = config.getPorts();
    int index = 0;
    for (std::list<int>::iterator it = ports.begin(); it != ports.end(); ++it) {
        int port = *it;
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
        _serverSocketToPort[serverSocketFd] = port; // 매핑 추가

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
                }
                else {  // 클라이언트 소켓인 경우
                    handleEvent(_pollFds[i].fd);
                    close(_pollFds[i].fd);
                    _pollFds.erase(_pollFds.begin() + i);
                    _clientSocketToPort.erase(_pollFds[i].fd); // 매핑 제거
                    --i;
                }
            }
        }
    }
}


void Service::handleEvent(int clientSocketFd) {
    char buffer[BUFFER_SIZE];
    int size = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);	// 클라이언트 소켓으로부터 Http 리퀘스트 내용 읽기
    buffer[size] = '\0';

    std::cout << std::endl << "========== Request ==========" << std::endl << std::endl;
    std::cout << buffer;
    std::cout << std::endl << "=============================" << std::endl << std::endl;

    HttpRequest httpRequest;
    httpRequest.parse(buffer);

    // 요청이 들어온 포트 번호를 가져옴
    int port = _clientSocketToPort[clientSocketFd];
    Server server = config.selectServer(httpRequest, port);

    // server clientMaxBodySize 가 0이 아닌 경우, body size 체크, 초과시 413
    if (server.clientMaxBodySize != 0 && httpRequest.body.size() > server.clientMaxBodySize) {
        HttpResponse httpResponse(server, httpRequest, 413);
        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
        std::cout << httpResponse.response;
        std::cout << std::endl << "=============================" << std::endl << std::endl;
        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        return;
    }

    // target에 해당하는 location 찾기, 없으면 404, 폴더로 끝나는 경우 index.html로 리다이렉트 (이후 수정 필요)
    if (httpRequest.target.back() == '/') {
        httpRequest.target = "/index.html";
    }
    Route route;
    for (std::list<Route>::const_iterator it = server.routes.begin(); it != server.routes.end(); it++) {
        if (it->location == httpRequest.target) {
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
        return;
    }

    // method가 허용되지 않으면 405
    if (!(route.acceptedHttpMethods & httpRequest.method)) {
        HttpResponse httpResponse(server, httpRequest, 405);
        std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
        std::cout << httpResponse.response;
        std::cout << std::endl << "=============================" << std::endl << std::endl;
        send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
        return;
    }

    HttpResponse httpResponse(server, httpRequest, 200);
    std::cout << std::endl << "========== Response =========" << std::endl << std::endl;
    std::cout << httpResponse.response;
    std::cout << std::endl << "=============================" << std::endl << std::endl;
    send(clientSocketFd, httpResponse.response.c_str(), httpResponse.response.size(), 0);
}











