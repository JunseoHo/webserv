#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <list>

class Config {
    private:
        std::list<int> ports;
        Config();
        Config(Config const &src);
        Config(&operator=(Config const &rhs));

    public:
        Config(char *conf);
        ~Config();
        bool readConfigFile(char *filename);
        std::list<int> getPorts() const;
};

#endif