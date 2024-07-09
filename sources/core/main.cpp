#include <list>
#include "../configuration/Config.hpp"
#include "Server.hpp"
int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	Config config(argv[1]);
	std::string root = "resources";

	Server server(config, root);
	server.Start();

    return 0;
}
