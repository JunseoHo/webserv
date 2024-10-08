#ifndef CORE_HPP
# define CORE_HPP

# include <list>
# include <string>
#include <ctime>
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
# include "BufferManager.hpp"
# include "SocketManager.hpp"
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Config.hpp"
# include "../utils/utils.h"
# include <signal.h>
# define BUFFER_SIZE 1024
# define TIME_LIMIT 3

struct cgiPidsInfo {
    int clientFd;
    pid_t pid;
    std::time_t startTime;
	std::string uri;
	HttpRequest request;
	Location location;
};

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
        std::vector<pollfd> _pollFds;

		SocketManager _socketManager;
        BufferManager _bufferManager;
        BufferManager _cgiBufferManager;
        BufferManager _responseBufferManager;
        std::vector <cgiPidsInfo> _cgiPidsInfo;

        void setUpSockets();
        void eventLoop();
        void handleEvent(int clientSocketFd);
        void executeCGI(std::string uri, HttpRequest &request, int clientSocketFd, Location location);
        void handleCgiEvent(int cgiPipeFd, int clientFd);
        void getMethod(std::string& uri, 
                        HttpRequest& httpRequest, const Location& location, int& statusCode, int& clientSocketFd);
        void postMethod(std::string& uri, HttpRequest& httpRequest, const Location& location, int& clientSocketFd);
        void deleteMethod(std::string& uri, HttpRequest& httpRequest, int& statusCode, int& clientSocketFd);
        void handleOutEvent(int clientSocketFd);
        void handleTimeoutCGI();
};

#endif
