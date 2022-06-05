//
// Created by Administrator on 2022/6/4.
//

#ifndef FM_UNPACK_H
#define FM_UNPACK_H


#include "TcpConnection.h"




class unpack {
public:
    using BufferCallBack = std::function<void(const TcpConnection::ptr &conn, Buffer &buffer, int n)>;
    using CharCallBack = std::function<void(const TcpConnection::ptr &conn, const char* data, int n)>;

    static const int PACKET_SIZE = 4;
    int headPos;
    void front(const std::shared_ptr<Buffer> &buffer); // 初始化头
    void encode(const std::shared_ptr<Buffer> &buffer,char* pos); // 编码
    void append(const std::shared_ptr<Buffer> &buffer,char*,size_t); // 编码
    static void adapterChar(const TcpConnection::ptr &conn, Buffer &buffer, int n,const CharCallBack& cb) ;
    static void adapterBuffer(const TcpConnection::ptr &conn, Buffer &buffer, int n,const BufferCallBack& cb) ;

    static void send(const TcpConnection::ptr &conn,char *data, std::size_t n);
};


#endif //FM_UNPACK_H
