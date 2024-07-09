#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <cstring>

#define TIMEOUT 3000
#define BUFFER_SIZE 1024

int main(void)
{
	/*

		struct pollfd
		{
			int fd; 		// 감시하고자 하는 파일 서술자
			short events; 	// 파일 서술자에 대해 감시하고자 하는 이벤트
			short revents; 	// 파일 서술자에서 발생한 이벤트
		}
	
	*/

	/*

		 events에서 감시할 수 있는 이벤트 리스트
		 
		 POLLIN: 파일에 읽을 수 있는 데이터가 저장될 때 발생하는 이벤트
		 POLLOUT: 파일에 데이터 쓰기가 가능할 때 발생하는 이벤트
		 POLLERR: 파일에 에러가 발생했을 때 발생하는 이벤트 (소켓의 오류, 연결 해제 등)
		 POLLHUP: 파일의 연결이 종료되었을 때 발생하는 이벤트
		 POLLNVAL: 파일 서술자가 유효하지 않을 때 발생하는 이벤트 (잘못된 파일 서술자 번호 등)
		
	*/

	struct pollfd io_event_monitor;

	// 표준 입력을 감시한다.
	io_event_monitor.fd = STDIN_FILENO;

	// 읽기 가능한 데이터의 저장을 감시
	io_event_monitor.events = POLLIN;

	std::cout << "Polling... You have to input any string within 3 seconds!" << std::endl;

	int result = poll(&io_event_monitor, 1, TIMEOUT);
	if (result == -1)
	{
		std::cerr << "Poll Failed: " << strerror(errno) << std::endl;
		return 1;
	}
	else if (result == 0)
	{
		std::cout << "Timeout!" << std::endl;
	}
	else 
	{
		// POLLIN 이벤트가 발생했다면 revents의 POLLIN에 해당하는 비트가 1로 반환된다.
		if (io_event_monitor.revents & POLLIN) {
			char buffer[BUFFER_SIZE];
            int size = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (size < 0) {
                std::cerr << "Read Failed: " << strerror(errno) << std::endl;
                return 1;
            }
            buffer[size] = '\0';
            std::cout << "Received input: " << buffer;
        }
	}

	return 0;
}