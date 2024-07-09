#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <unistd.h>  

/*
	자세한 주석은 server.cpp를 참고하라.
*/


const int PORT = 3000;
const int BUFFER_SIZE = 1024;

int	main(void)
{
	// 클라이언트 소켓 생성
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
        std::cerr << "Socket Creation Failed: " << strerror(errno) << std::endl;
        return 1;
    }

	std::cout << "ClientSocket Created" << std::endl;

	// 주소 생성
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	// 서버 소켓에 연결 요청
	if (connect(clientSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "Server Connection Failed: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
	}

	std::cout << "ServerSocket Connected" << std::endl;

	// 연결된 서버 소켓에 데이터 전송
	const char* message = "Hello, webserv!";
	if (send(clientSocket, message, strlen(message), 0) < 0)
	{
		std::cerr << "Data Sending Failed: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
	}

	std::cout << "Message Sent To The Server: " << message << std::endl;

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

    int size = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (size < 0) {
        std::cerr << "Receiving Data Failed" << std::endl;
        close(clientSocket);
        return 1;
    } else if (size == 0) {
        std::cerr << "ServerSocket Disconnected" << std::endl;
        close(clientSocket);
        return 1;
    }

	std::cout << "Received Message From Server: " << buffer << std::endl;

	// 클라이언트 소켓 종료
	if (close(clientSocket) < 0) {
        std::cerr << "Socket Closing Failed: " << strerror(errno) << std::endl;
        return 1;
    }

	std::cout << "Socket Closed" << std::endl;

	return 0;
}