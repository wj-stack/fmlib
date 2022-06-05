//
// Created by Administrator on 2022/6/4.
//

#include <netinet/in.h>
#include "unpack.h"

static void intToBytes(int value,char src[4])
{
    value = ntohl(value);
    memcpy(src, &value, sizeof(value));
}

static int bytesToInt(const char src[4]) {
    int value;
    memcpy(&value, src, sizeof(value));
    value = htonl(value);
    return value;
}


void unpack::front(const std::shared_ptr<Buffer> &buffer) {
    char n[PACKET_SIZE];
    buffer->append(n, PACKET_SIZE);
    headPos = buffer->getWriteIndex() - PACKET_SIZE;
}

void unpack::encode(const std::shared_ptr<Buffer> &buffer,char* pos) {
    intToBytes(buffer->getReadable() - PACKET_SIZE, pos);
}

void unpack::append(const std::shared_ptr<Buffer> &buffer, char * data, size_t n) {
    buffer->append(data, n);
    encode(buffer, buffer->begin() + headPos);
}

void unpack::adapterBuffer(const TcpConnection::ptr &conn, Buffer &buffer, int n, const unpack::BufferCallBack &cb) {
    while (buffer.getReadable() >= PACKET_SIZE) {
        int size = bytesToInt(buffer.begin() + buffer.getReadIndex());
        if (buffer.getReadable() >= size + PACKET_SIZE)
        {
            if(cb)cb(conn, buffer, size);
            buffer.retrieve(size + PACKET_SIZE);
        }else break;
    }
}

void unpack::adapterChar(const TcpConnection::ptr &conn, Buffer &buffer, int n, const unpack::CharCallBack &cb) {
    spdlog::info("read: n {0} buffer.getReadable:{1}", n, buffer.getReadable());
    while (buffer.getReadable() >= PACKET_SIZE) {
        int size = bytesToInt(buffer.begin() + buffer.getReadIndex());
        if (buffer.getReadable() >= size + PACKET_SIZE)
        {
            if(cb)cb(conn, buffer.begin() + buffer.getReadIndex() + unpack::PACKET_SIZE, size);
            buffer.retrieve(size + PACKET_SIZE);
        } else break;
    }
}

void unpack::send(const TcpConnection::ptr &conn, char *data, std::size_t n) {
    auto buffer = std::make_shared<Buffer>();
    unpack unpack{};
    unpack.front(buffer);
    unpack.append(buffer, data, n);
    conn->Send(buffer);
}



