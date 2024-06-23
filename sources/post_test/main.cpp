#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#define PORT 51000
#define SERVER_ADDR "127.0.0.1"

void sendPostRequest(const std::string& target, const std::string& body) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error" << std::endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed" << std::endl;
        return;
    }

    std::string request = "POST " + target + " HTTP/1.1\r\n";
    request += "Host: " + std::string(SERVER_ADDR) + ":" + std::to_string(PORT) + "\r\n";
    request += "Content-Type: text/plain\r\n";
    request += "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n";
    request += body;

    send(sock, request.c_str(), request.length(), 0);
    std::cout << "POST request sent\n" << request << std::endl;

    read(sock, buffer, 1024);
    std::cout << "Response:\n" << buffer << std::endl;

    close(sock);
}

int main() {
    std::string target = "/testfile.txt";
    std::string body = "This is a test POST request body.";
    sendPostRequest(target, body);
    return 0;
}