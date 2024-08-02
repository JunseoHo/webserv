#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP
# include <map>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <unistd.h>
# define BACK_LOG 10

class SocketManager
{
	public:
		SocketManager();
		SocketManager(std::vector<int> ports);
		SocketManager& operator= (const SocketManager& rhs);
		~SocketManager();

		const std::map<int, int>& GetSocketFdPortMap(void) const;

	private:
		SocketManager(const SocketManager& other);

		std::map<int, int> _socketFdPortMap;
};

#endif