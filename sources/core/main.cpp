#include <list>
#include "Server.hpp"

int main(int argc, char *argv[])
{
	// 설정 초기화 (추후 Config 파일로 대체)
    std::list<int> ports;
	readConfigFile(ports);
	// int port1 = 51000;
	// int port2 = 52000;
	// int port3 = 53000;
    // ports.push_back(port1);
    // ports.push_back(port2);
    // ports.push_back(port3);

	std::string root = "resources";

	// 서버 구동
	Server server(ports, root);
	server.Start();

    return 0;
}
