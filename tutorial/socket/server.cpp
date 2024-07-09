#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 3000;
const int BUFFER_SIZE = 1024;


int	main(void)
{
	// https://man7.org/linux/man-pages/man2/socket.2.html
	// socket 함수는 소켓의 파일 서술자를 반환한다.
	// 리눅스 커널의 소켓은 sock 확장자를 가지는 파일로 구현되므로 파일 서술자를 통해 접근할 수 있다.
	// AF_INET은 IPv4를 뜻하며 SOCK_STREAM 방식을 사용한다.
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		std::cerr << "Socket Creation Failed: " << strerror(errno) << std::endl;
        return 1;
	}

	std::cout << "Server Socket Created" << std::endl;

	// sockeaddr_in은 C, C++에서 주소를 표현할 때 사용하는 구조체이다.
	sockaddr_in serverAddr, clientAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	socklen_t clientAddrLen = sizeof(clientAddr);

	// 소켓과 주소를 바인딩한다.
	// 만약 해당 주소가 사용 중이라면 Address already in use 예외가 발생한다.
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cerr << "Address Binding Failed: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
	}

	std::cout << "Socket Bound With Address" << std::endl;

	// 소켓을 수신 대기 상태로 전환한다.
	// 함께 전달되는 int 인수는 대기열의 길이이다.
	if (listen(serverSocket, 5) < 0)
	{
		std::cerr << "Socket Listening Failed: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
	}

	std::cout << "Server is listening..." << std::endl;

	// 클라이언트 소켓이 연결을 요청할 경우 이를 수락한다.
	// 두번째, 세번째 인수는 연결을 요청한 클라이언트의 정보를 받아오기 위해 필요하다.
	int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);

	std::cout << "Connect is accepted!" << std::endl;

	char buffer[BUFFER_SIZE];

	// 버퍼를 Null로 초기화한다.
	memset(buffer, 0, BUFFER_SIZE);

	// recv 함수를 통해 클라이언트에서 전송한 데이터를 버퍼로 받아올 수 있다.
    int size = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (size < 0) {
        std::cerr << "Receiving Data Failed" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return 1;
    } else if (size == 0) {
        std::cerr << "ClientSocket Disconnected" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    std::cout << "Received Data From Client: " << buffer << std::endl;

	// send 함수를 통해 클라이언트에게 데이터를 전송할 수 있다.
	const char* message = "Hello, webclien!";
	if (send(clientSocket, message, strlen(message), 0) < 0)
	{
		std::cerr << "Data Sending Failed: " << strerror(errno) << std::endl;
        close(clientSocket);
        return 1;
	}

	std::cout << "Message Sent To The Client: " << message << std::endl;

	if (close(clientSocket) < 0 || close(serverSocket))
	{
		std::cerr << "Socket Closing Failed: " << strerror(errno) << std::endl;
        return 1;
	}

	std::cout << "Socket Closed" << std::endl;

    return 0;
}