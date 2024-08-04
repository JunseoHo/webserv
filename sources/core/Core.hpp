#ifndef CORE_HPP
# define CORE_HPP

# include <list>
# include <string>
# include <vector>
# include <map>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <cstring>
# include <poll.h>
# include <fstream>
# include <filesystem>
# include <iostream>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "SocketManager.hpp"
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Config.hpp"
# include "../utils/utils.h"
# define BUFFER_SIZE 1024


class Core {
    public:
        Core(const Config &config);
		~Core();
        
        Config config;

		void Start();

    private:
        Core();
        Core(const Core& other);
        Core& operator= (const Core& rhs);

		SocketManager _socketManager;

        void setUpSockets();
        void eventLoop();
        void handleEvent(int clientSocketFd);
        void executeCGI(const std::string& uri, HttpRequest &request, int clientSocketFd, Location location);
        void handleCgiEvent(int cgiPipeFd, int clientFd);
        void getMethod(std::string& uri, 
                        HttpRequest& httpRequest, const Location& location, int& statusCode, int& clientSocketFd, Server& server);
        void postMethod(std::string& uri, HttpRequest& httpRequest, const Server& server, const Location& location, int& clientSocketFd);
        void deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd);

    
        std::vector<pollfd> _pollFds;
		std::map<int, std::string> _bufferTable;
        std::map<int, std::string> _cgiBufferTable;
		std::map<int, int> _serverSocketToPort;
		std::map<int, int> _clientSocketToPort;
        std::map<int, int> _cgiFdToClientFd;
		std::map<int, bool> _cgiChunked;
        std::map<int, int> _clientFdToCgiFd;
};

#endif
