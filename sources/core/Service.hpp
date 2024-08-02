#ifndef SERVICE_HPP
# define SERVICE_HPP

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
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Config.hpp"
# include "../utils/utils.h"
# define BUFFER_SIZE 1024

class Service {
    public:
        Service(const Config &config);
        void Start();
        ~Service();
        Config config;

    private:
        Service();
        Service(const Service& obj);
        Service& operator= (const Service& rhs);

        void setNonBlocking(int fd);
        void setupSockets();
        void eventLoop();
        void handleEvent(int clientSocketFd);
        void executeCGI(const std::string& uri, HttpRequest &request, int clientSocketFd, Location location);
        void handleCgiEvent(int cgiPipeFd, int clientFd);
        void getMethod(std::string& uri, 
                        HttpRequest& httpRequest, const Location& location, int& statusCode, int& clientSocketFd, Server& server);
        void postMethod(std::string& uri, HttpRequest& httpRequest, const Server& server, const Location& location, int& clientSocketFd);
        void deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd);

    
		std::map<int, std::string> _bufferTable;
        std::map<int, std::string> _cgiBufferTable;
        std::vector<pollfd> _pollFds;
        std::vector<int> _serverSocketFds;
        static const int _backLog;
		std::map<int, int> _serverSocketToPort;
		std::map<int, int> _clientSocketToPort;
        std::map<int, int> _cgiFdToClientFd;
		std::map<int, bool> _cgiChunked;
        std::map<int, int> _clientFdToCgiFd;
};

#endif
