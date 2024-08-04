#ifndef UTILS_H
# define UTILS_H
# include <iostream>
# include <sys/stat.h>
# include <dirent.h>
# include <fcntl.h>
# include <unistd.h>
# include <fstream>
# include <filesystem>
# include <vector>

bool isDirectory(const std::string& path);
std::string getIndex(const std::string& path);
bool endsWith(const std::string& str, const std::string& suffix);
void setNonBlocking(int fd);
std::string getHeaderValue(const std::string &header, const std::string &key);
std::vector<std::string> split(const std::string &str, const std::string &delim);
void parse_multipart_data(const std::string& path, const std::string& body, const std::string& boundary);
std::string readFileToString(const std::string& filename);
std::string listDirectory(const std::string& path);

#endif