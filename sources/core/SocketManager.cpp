#include "SocketManager.hpp"

SocketManager::SocketManager() { /* DO NOTHING */}
SocketManager::~SocketManager() { /* DO NOTHING */}
SocketManager::SocketManager(const SocketManager& other) { /* THIS IS PRIVATE */ }

SocketManager::SocketManager(std::vector<int> ports)
{
    for (int j = 0; j < ports.size(); j++) {
        int port = ports[j];
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << std::endl;
            close(fd);
            continue; // 소켓 설정 중 에러가 발생하더라도 다른 소켓의 개방은 계속 진행한다.
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
        {
            std::cerr << "bind failed with error: " << strerror(errno) << std::endl;
            close(fd);
            continue;
        }

        if (listen(fd, BACK_LOG) == -1)
        {
            std::cerr << "listen failed with error: " << strerror(errno) << std::endl;
            close(fd);
            continue;
        }

        setNonBlocking(fd);
        addServerSockerFd(fd);
        _socketFdPortMap[fd] = port;
        std::cout << "Port " << port << " is listening." << std::endl;
    }
}

SocketManager& SocketManager::operator= (const SocketManager& rhs)
{
	if (this != &rhs)
	{
		_socketFdPortMap = rhs.GetSocketFdPortMap();
        _serverSocketFds = rhs.GetServerSocketFds();
	}
	return *this;
}

void SocketManager::addServerSockerFd(int fd)
{
    // _serverSocketFds에 이미 fd가 있으면 추가하지 않는다.
    if (_serverSocketFds.end() == std::find(_serverSocketFds.begin(), _serverSocketFds.end(), fd))
        _serverSocketFds.push_back(fd);
}

void SocketManager::addClientSocketFd(int fd, int port)
{
    // _clientSocketFds에 이미 fd가 있으면 추가하지 않는다.
    if (_clientSocketFds.end() == std::find(_clientSocketFds.begin(), _clientSocketFds.end(), fd))
    {
        _socketFdPortMap[fd] = port;
        _clientSocketFds.push_back(fd);
    }
}

void SocketManager::removeClientSocketFd(int fd)
{
    _clientSocketFds.erase(std::remove(_clientSocketFds.begin(), _clientSocketFds.end(), fd), _clientSocketFds.end());
    _socketFdPortMap.erase(fd);
}

const std::map<int, int>& SocketManager::GetSocketFdPortMap(void) const
{
	return _socketFdPortMap;
}

const std::vector<int>& SocketManager::GetServerSocketFds(void) const
{
    return _serverSocketFds;
}

int SocketManager::GetPortBySocketFd(int fd) const
{
    if (_socketFdPortMap.end() == _socketFdPortMap.find(fd))
        return -1;
    return _socketFdPortMap.at(fd);
}

bool SocketManager::isServerSocketFd(int fd) const
{
    return _serverSocketFds.end() != std::find(_serverSocketFds.begin(), _serverSocketFds.end(), fd);
}