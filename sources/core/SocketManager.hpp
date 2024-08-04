#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP
# include <map>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <unistd.h>
# include "../utils/utils.h"

# define BACK_LOG 10

class SocketManager
{
	public:
		SocketManager();
		SocketManager(std::vector<int> ports);
		SocketManager& operator= (const SocketManager& rhs);
		~SocketManager();

		void addServerSockerFd(int fd);
		void addClientSocketFd(int fd, int port);
		void removeClientSocketFd(int fd);
		const std::map<int, int>& GetSocketFdPortMap(void) const;
		const std::vector<int>& GetServerSocketFds(void) const;

		bool isServerSocketFd(int fd) const;
		int	 GetPortBySocketFd(int fd) const;

	private:
		SocketManager(const SocketManager& other);

		std::map<int, int> _socketFdPortMap;
		std::vector<int> _serverSocketFds;
		std::vector<int> _clientSocketFds;
};

#endif