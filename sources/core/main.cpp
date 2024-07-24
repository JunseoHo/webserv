#include <list>
#include "../config/Config.hpp"
#include "Service.hpp"


int main(int argc, char *argv[]) {
	if (!validateConfig(argc, argv)) //too many arguments
		return 1;
	std::string configPath = getConfig(argc, argv);
	try
	{
		Config config(configPath);
		// config.print();
		Service service(config, ROOT);
		service.Start();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
    return 0;
}
