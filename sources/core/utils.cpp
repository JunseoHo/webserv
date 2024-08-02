#include <fcntl.h>
#include <iostream>

void setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, FD_CLOEXEC | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL failed with error: " << strerror(errno) << std::endl;
    }
}