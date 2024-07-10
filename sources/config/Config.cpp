#include "Config.hpp"

Config::Config()
{
	/* DO NOTHING */
}

Config::Config(const Config& other)
{
	/* DO NOTHING */
}

Config::Config(std::string& configFilePath)
{
	/*
		Parse file from here.
		If file is invalid, throw InvalidConfigException.
	*/
}

Config::~Config()
{
	/* DO NOTHING */
}

const char* Config::InvalidConfigFormatException::what() const throw()
{
	return "Configuration file format is incorrect."
}