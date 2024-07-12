#include "Server.hpp"
const int Service::_backLog = 10;
Service::Service(const Service& obj) { /* DO NOTHING */ }
Service::Service() { /* DO NOTHING */ }
Service::~Service() { /* DO NOTHING */ }
Service& Service::operator= (const Service& rhs) { /* DO NOTHING */ }

Service::Service(const Config &config, const std::string& resourcesPath)
    : _ports(config.getPorts()), _resourcesPath(resourcesPath), config(config) {
    _pollFds.resize(config.getPorts().size());
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.substr(str.length() - suffix.length()) == suffix;
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
    int index = 0;
    for (std::list<int>::iterator it = _ports.begin(); it != _ports.end(); ++it) {
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
                }
                else {  // 클라이언트 소켓인 경우
                    handleEvent(_pollFds[i].fd);
                    close(_pollFds[i].fd);
                    _pollFds.erase(_pollFds.begin() + i);
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
    Server server = config.selectServer(httpRequest);

    std::cout << server.host << "@:@" << server.port << std::endl;

    std::string response = "";
    if (httpRequest.method == GET) {
        std::string responseBody = readFileToString(_resourcesPath + httpRequest.target);
        if (endsWith(httpRequest.target, ".svg"))
        {
            response = "HTTP/1.1 200 OK\r\nContent-Type: image/svg+xml\r\nContent-Length: "
                + std::to_string(responseBody.length()) + "\r\n\r\n"
                + responseBody;
        }
        else
        {
            response = "HTTP/1.1 200 OK\r\nContent-Length: "
                + std::to_string(responseBody.length()) + "\r\n\r\n"
                + responseBody;
        }
        send(clientSocketFd, response.c_str(), response.size(), 0);
    } else if (httpRequest.method == POST) {
        std::string requestBody = httpRequest.body;
        std::string filePath = "post" + httpRequest.target;
        std::ofstream outfile(filePath);
        if (outfile.is_open()) {
            outfile << requestBody;
            outfile.close();
            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        } else {
            response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        }
    }
}











