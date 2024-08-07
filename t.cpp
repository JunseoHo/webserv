#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int get_pending_connections(int listen_fd) {
    int pending_connections = 0;
    socklen_t pending_len = sizeof(pending_connections);

    // SO_RCVBUF 옵션을 사용하여 대기 중인 데이터의 크기를 확인
    if (getsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, &pending_connections, &pending_len) == -1) {
        perror("getsockopt");
        return -1;
    }

    return pending_connections;
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    int pending_connections = get_pending_connections(listen_fd);
    if (pending_connections != -1) {
        std::cout << "Pending connections: " << pending_connections << std::endl;
    }

    close(listen_fd);
    return 0;
}
