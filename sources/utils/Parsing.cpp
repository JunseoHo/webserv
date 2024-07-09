#include "Utils.hpp"

bool readConfigFile(std::list<int> &ports) {
	std::ifstream file(CONFIG);
	if (!file.is_open()) {
		std::cerr << "Failed to open config file" << std::endl;
		return false;
	}

	std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.find("listen:") == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                // 포트 번호 추출
                std::string portStr = line.substr(colonPos + 1);
                portStr.erase(0, portStr.find_first_not_of(" \t"));
                portStr.erase(portStr.find_last_not_of(" \t") + 1);
                
                // 정수형 포트 번호로 변환하여 저장
                int port = std::atoi(portStr.c_str());
                ports.push_back(port);
            }
        }
    }
    file.close();
    return true;
}