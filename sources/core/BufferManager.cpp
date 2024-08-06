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
    if (isBufferEmpty(fd))
        addBuffer(fd);
    _bufferTable[fd] += std::string(data, size);
}

void BufferManager::removeBufferFront(int fd, ssize_t size)
{
    _bufferTable[fd].erase(0, size);
    if (_bufferTable[fd].empty())
        removeBuffer(fd);
}

const std::string& BufferManager::GetBuffer(int fd) const
{
    if (isBufferEmpty(fd))
        throw std::runtime_error("BufferManager::GetBuffer: Buffer is empty.");
    return _bufferTable.at(fd);
}

std::string BufferManager::GetSubBuffer(int fd, ssize_t size) const
{
    if (isBufferEmpty(fd))
        throw std::runtime_error("BufferManager::GetSubBuffer: Buffer is empty.");
    if (size > _bufferTable.at(fd).size())
        size = _bufferTable.at(fd).size();
    return _bufferTable.at(fd).substr(0, size);
}

bool BufferManager::isBufferEmpty(int fd) const
{
    return (_bufferTable.find(fd) == _bufferTable.end());
}

const std::map<int, std::string>& BufferManager::GetBufferTable(void) const
{
    return _bufferTable;
}