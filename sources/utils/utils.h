#ifndef UTILS_H
# define UTILS_H
# include <iostream>
# include <sys/stat.h>
# include <dirent.h>
# include <fcntl.h>

bool isDirectory(const std::string& path);
std::string getIndex(const std::string& path);
bool endsWith(const std::string& str, const std::string& suffix);
void setNonBlocking(int fd);

#endif