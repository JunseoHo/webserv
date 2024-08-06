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

void setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, FD_CLOEXEC | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL failed with error: " << strerror(errno) << std::endl;
    }
}

// 헤더에서 원하는 항목을 추출하는 함수
std::string getHeaderValue(const std::string &header, const std::string &key)
{
    std::string value = "";
    std::size_t pos = header.find(key);
    if (pos != std::string::npos)
    {
        pos += key.length();
        while (header[pos] == ' ' || header[pos] == ':')
            pos++;
        while (header[pos] != '\r' && header[pos] != '\n')
        {
            value += header[pos];
            pos++;
        }
    }
    return value;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

void parse_multipart_data(const std::string& path, const std::string& body, const std::string& boundary) {
    std::vector<std::string> parts = split(body, boundary);
    for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
        std::string part = *it;
        if (part.empty() || part == "--\r\n" || part == "--") {
            continue;
        }

        // 헤더와 바디 추출
        size_t header_end_pos = part.find("\r\n\r\n");
        if (header_end_pos == std::string::npos) {
            continue;
        }
        std::string headers = part.substr(0, header_end_pos);
        std::string body = part.substr(header_end_pos + 4);

        // 파일이름 추출
        size_t cd_pos = headers.find("Content-Disposition:");
        if (cd_pos == std::string::npos) {
            continue;
        }

        std::string content_disposition = headers.substr(cd_pos, headers.find("\r\n", cd_pos) - cd_pos);
        size_t filename_pos = content_disposition.find("filename=");
        if (filename_pos != std::string::npos) {
            std::string filename = content_disposition.substr(filename_pos + 10);
            filename = filename.substr(0, filename.find("\""));

            // Save file
            std::ofstream outfile(path + filename, std::ios::binary);
            outfile.write(body.data(), body.size());
            outfile.close();

            std::cout << "Saved file: " << filename << std::endl;
        }
    }
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename.substr(1));
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

std::string listDirectory(const std::string& path) {
    // 경로 문자열의 첫 글자를 제거
    std::string modified_path = path.substr(1);

    DIR *dir = opendir(modified_path.c_str());
    if (dir == NULL) {
        std::cerr << "Error opening directory: " << std::strerror(errno) << std::endl;
        return "";
    }

    struct dirent *entry;
    std::string file_list;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore '.' and '..' entries
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            file_list += entry->d_name;
            file_list += "\n";
        }
    }

    closedir(dir);
    return file_list;
}