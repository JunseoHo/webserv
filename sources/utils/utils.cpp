#include "utils.h"

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool isDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR)
            return true;
    }
    return false;
}

std::string getIndex(const std::string& path) {
    DIR *dir = opendir(path.c_str());
    std::string index = "";
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore '.' and '..' entries
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            if (endsWith(entry->d_name, ".html")) {
                index = entry->d_name;
                break;
            }
        }
    }

    closedir(dir);
    return index;
}