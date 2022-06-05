//
// Created by wyatt on 2022/4/21.
//

#include <netinet/in.h>
#include "Buffer.h"
#include "spdlog/spdlog.h"

ssize_t Buffer::read(int fd) {
//    WYATT_LOG_ROOT_DEBUG() << "read fd: " << fd;
    char extraBuff[65535];
    struct iovec vec[2];
    vec[0].iov_base = begin() + writeIndex;
    vec[0].iov_len = getWritable();
    vec[1].iov_base = extraBuff;
    vec[1].iov_len = sizeof(extraBuff);
    ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        spdlog::error("buffer read error: {}", errno);
    } else if (n <= getWritable()) {
        writeIndex += (int) n;
    } else {
//        spdlog::info("buffer not ok : {}", n);
        int old = getWritable();
        writeIndex = (int) buf.size();
        append(extraBuff, (int) n - old);
    }

    return n;
}

void Buffer::append(char *data, uint32_t n) {
    if (getWritable() >= n)
    {
        // 可以直接写入
//        spdlog::info("可以直接写入");

    }else if ((readIndex - preAppendIndex) + (buf.size() - writeIndex) >= n) {
//        spdlog::info("空间够用，只要把久数据移动到前面，然后再添加进来");
        // 空间够用，只要把久数据移动到前面，然后再添加进来
        std::copy(begin() + readIndex, begin() + writeIndex, begin() + preAppendIndex);
        writeIndex = preAppendIndex + getReadable();
        readIndex = preAppendIndex;
    } else {
//        spdlog::info("空间不够用 resize：{}",writeIndex + n);
        // 空间不够用
        buf.resize(writeIndex + n);

    }
    std::copy(data, data + n, begin() + writeIndex);
    writeIndex += n;

}

void Buffer::retrieve(int len) {
    if (len <= getReadable()) readIndex += len;
    if (readIndex == writeIndex) {
        writeIndex = readIndex = preAppendIndex;
    }
}

void Buffer::CodeSize(uint32_t n) {
    n = ntohl(n);
    memcpy(buf.data(), &n, BUFFER_PREAPPEND);
}

uint32_t Buffer::GetCodeSize() {
    uint32_t n;
    memcpy(&n, buf.data(), sizeof(uint32_t));
    n = htonl(n);
    return n;
}
