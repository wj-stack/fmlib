//
// Created by wyatt on 2022/4/21.
//

#ifndef SOCKETFRAMEWORK_BUFFER_H
#define SOCKETFRAMEWORK_BUFFER_H

#include <vector>
#include <sys/socket.h>
#include <sys/uio.h>

//#define BUFFER_TYPE uint32_t
#define BUFFER_PREAPPEND 8

#include <atomic>
//#include <spdlog/spdlog.h>
//static std::atomic_int cnt{0};
class Buffer {

private:
    std::vector<char> buf;
    int writeIndex;
    int preAppendIndex;
    int readIndex;

public:
    explicit Buffer(int preAppend = BUFFER_PREAPPEND, int size = 1024) : buf(size), preAppendIndex(preAppend), readIndex(preAppend),
                                                 writeIndex(preAppend) {
//        cnt++;
//        spdlog::info("buffer cnt: {}", cnt);
    }
//    ~Buffer()
//    {
//        cnt--;
//        spdlog::info("buffer cnt: {}", cnt);
//    }

    char *begin() { return buf.data(); }

    int getWritable() const { return buf.size() - writeIndex; }

    int getReadable() const { return writeIndex - readIndex; }

    void retrieve(int len); // 给用户调用，取回数据后要修改readIndex

    ssize_t read(int fd);

    void append(char *data, uint32_t n);

    size_t getCapacity() const
    {
        return buf.capacity();
    }

    int getReadIndex() const {
        return readIndex;
    }

    void setReadIndex(int i) {
        Buffer::readIndex = i;
    }

    int getWriteIndex() const {
        return writeIndex;
    }

    void setWriteIndex(int i) {
        Buffer::writeIndex = i;
    }

    void CodeSize(uint32_t);
    uint32_t GetCodeSize();
};


#endif //SOCKETFRAMEWORK_BUFFER_H
