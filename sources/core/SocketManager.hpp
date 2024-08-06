#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP
# include <map>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <unistd.h>
# include <poll.h>
# include "../utils/utils.h"

# define BACK_LOG 20

class SocketManager
{
	public:
		SocketManager();
		SocketManager(std::vector<int> ports);
		SocketManager& operator= (const SocketManager& rhs);
		~SocketManager();

		pollfd newClientPollfd(int serverFd);

		void addServerSockerFd(int fd);
		void addClientSocketFd(int fd, int port);
		void removeClientSocketFd(int fd);

		void connectCgiToClient(int cgiFd, int clientFd);
		void disconnectCgiToClient(int cgiFd);
		
		const int GetClientFdByCgiFd(int cgiFd) const;
		const int GetCgiFdByClientFd(int clientFd) const;
		int	GetPortBySocketFd(int fd) const;

		const std::map<int, int>& GetSocketFdPortMap(void) const;
		const std::map<int, int>& GetCgiFdToClientFdMap(void) const;
		const std::map<int, int>& GetClientFdToCgiFdMap(void) const;
		const std::vector<int>& GetServerSocketFds(void) const;
		const std::vector<int>& GetClientSocketFds(void) const;

		bool isServerSocketFd(int fd) const;
		bool isConnectedCgiToClient(int cgiFd) const;
		bool isConnectedClinetToCgi(int clientFd) const;

	private:
		SocketManager(const SocketManager& other);

		std::map<int, int> _socketFdPortMap;
		std::map<int, int> _cgiFdToClientFd;
        std::map<int, int> _clientFdToCgiFd;
		std::vector<int> _serverSocketFds;
		std::vector<int> _clientSocketFds;
};

#endif