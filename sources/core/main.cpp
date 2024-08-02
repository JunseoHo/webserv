#include <list>
#include "../config/Config.hpp"
#include "Service.hpp"
#include "webserv.hpp"


int main(int argc, char *argv[]) {
	if (argc > 2)
	{
		std::cerr << "Too many arguments" << std::endl;
		return 1;
	}

	std::string configPath = (argc == 1) ? DEFAULT_CONFIG_PATH : argv[1];

	try
	{
		Config config(configPath);
		std::cout << config;

		std::cout << "webserv is running..." << std::endl;

		Service service(config);
		service.Start();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
    return 0;
}
