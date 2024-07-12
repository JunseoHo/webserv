#include <list>
#include "../config/Config.hpp"
#include "Service.hpp"
int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}
	std::string configPath = argv[1];
	Config config(configPath);
	config.print();
	std::string root = "resources";

	Service service(config, root);
	service.Start();

    return 0;
}
