#include "SocketManager.hpp"
#include "utils.cpp"

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
        _socketFdPortMap[fd] = port;

        std::cout << "Port " << port << " is listening." << std::endl;
    }
}

SocketManager& SocketManager::operator= (const SocketManager& rhs)
{
	if (this != &rhs)
	{
		_socketFdPortMap = rhs.GetSocketFdPortMap();
	}
	return *this;
}

const std::map<int, int>& SocketManager::GetSocketFdPortMap(void) const
{
	return _socketFdPortMap;
}