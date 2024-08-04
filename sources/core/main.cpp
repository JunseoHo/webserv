#include <list>
#include "../config/Config.hpp"
#include "webserv.hpp"

int main(int argc, char *argv[]) {
	if (argc > 2)
	{
		std::cerr << "Too many arguments" << std::endl;
		return 1;
	}


	try
	{
		Config config((argc == 1) ? DEFAULT_CONFIG_PATH : argv[1]);
		std::cout << config;

		Core core(config);
		core.Start();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
    return 0;
}
