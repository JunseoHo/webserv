#ifndef UTILS_H
# define UTILS_H
# include <sys/stat.h>
# include <iostream>
# include <dirent.h>

bool isDirectory(const std::string& path);
std::string getIndex(const std::string& path);
bool endsWith(const std::string& str, const std::string& suffix);
#endif