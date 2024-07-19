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
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Config.hpp"
# define BUFFER_SIZE 1024
# define ROOT "/Users/jho/Desktop/webserv/resources/"

class Service {
    public:
        Service(const Config &config, const std::string& resourcesPath);
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
        bool handleEvent(int clientSocketFd);
    
		std::map<int, std::string> _bufferTable;
        std::string _resourcesPath;
        std::vector<pollfd> _pollFds;
        std::vector<int> _serverSocketFds;
        static const int _backLog;
		std::map<int, int> _serverSocketToPort;
		std::map<int, int> _clientSocketToPort;
};

#endif
