#include <list>
#include "../config/Config.hpp"
#include "Service.hpp"

bool ValidateArgs(int argc, char *argv[]) {
	if (argc > 2) {
		std::cerr << "*no config file*\nUsage: ./webserv <config_file>" << std::endl;
		std::exit(1);
	}
	else if (argc == 1)
		return false;
	return true;
}

int main(int argc, char *argv[]) {
	std::string configPath;
	if (!ValidateArgs(argc, argv))
		configPath = "conf/default.yml";
	else
		configPath = argv[1];
	Config config(configPath);
	config.print();
	Service service(config, ROOT);
	service.Start();
    return 0;
}
