#ifndef BUFFER_MANAGER_HPP
# define BUFFER_MANAGER_HPP
# include <map>
# include <string>
# include <iostream>
# include "../utils/utils.h"

# define BACK_LOG 10

class BufferManager
{
	public:
		BufferManager();
		BufferManager& operator= (const BufferManager& rhs);
		~BufferManager();

        void addBuffer(int fd);
        void removeBuffer(int fd);
        void appendBuffer(int fd, const char *data, ssize_t size);
        void removeBufferFront(int fd, ssize_t size);

        const std::string& GetBuffer(int fd) const;
        const std::string& GetSubBuffer(int fd, ssize_t size) const;

        const std::map<int, std::string>& GetBufferTable(void) const;

        bool isBufferEmpty(int fd) const;

	private:
		BufferManager(const BufferManager& other);

        std::map<int, std::string> _bufferTable;
};

#endif