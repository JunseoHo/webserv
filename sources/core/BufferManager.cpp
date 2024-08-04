#include "BufferManager.hpp"

BufferManager::BufferManager() { /* DO NOTHING */}
BufferManager::~BufferManager() { /* DO NOTHING */}
BufferManager::BufferManager(const BufferManager& other) { /* THIS IS PRIVATE */ }

BufferManager& BufferManager::operator= (const BufferManager& rhs)
{
    if (this != &rhs)
    {
        _bufferTable = rhs.GetBufferTable();
    }
    return *this;
}

void BufferManager::addBuffer(int fd)
{
    _bufferTable[fd] = "";
}

void BufferManager::removeBuffer(int fd)
{
    _bufferTable.erase(fd);
}

void BufferManager::appendBuffer(int fd, const char *data, ssize_t size)
{
    _bufferTable[fd] += std::string(data, size);
}

void BufferManager::removeBufferFront(int fd, ssize_t size)
{
    _bufferTable[fd] = _bufferTable[fd].substr(size);
}

const std::string& BufferManager::GetBuffer(int fd) const
{
    if (isBufferEmpty(fd))
        throw std::runtime_error("BufferManager::GetBuffer: Buffer is empty.");
    return _bufferTable.at(fd);
}

bool BufferManager::isBufferEmpty(int fd) const
{
    return _bufferTable.find(fd) == _bufferTable.end();
}

const std::map<int, std::string>& BufferManager::GetBufferTable(void) const
{
    return _bufferTable;
}