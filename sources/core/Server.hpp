#ifndef SERVER_HPP
# define SERVER_HPP

# define BUFFER_SIZE 1024
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

class Service {
    public:
        Service(const Config &config, const std::string& resourcesPath);
        void Start();
        ~Service();

    private:
        std::list<int> _ports;
        Config config;
        Service();
        Service(const Service& obj);
        Service& operator= (const Service& rhs);

        void setNonBlocking(int fd);
        void setupSockets();
        void eventLoop();
        void handleEvent(int clientSocketFd);
    
        std::string _resourcesPath;
        std::vector<pollfd> _pollFds;
        std::vector<int> _serverSocketFds;
        static const int _backLog;
};

#endif
