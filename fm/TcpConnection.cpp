//
// Created by wyatt on 2022/4/21.
//

#include "TcpConnection.h"
#include "Socket.h"
#include <unistd.h>

TcpConnection::TcpConnection(EventLoop *loop_, int fd_, int id_) : loop(loop_), fd(fd_), channel(loop, fd), id(id_) {

//    WYATT_LOG_ROOT_DEBUG() << "TcpConnection";

}

void TcpConnection::Send(const std::string &message) {
    if(isConnected())
    {
        if (loop->isInLoopThread())
        {
            sendInLoop(message.c_str(),message.size());
        }else
        {
            loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message.c_str(),message.size()));
        }
    }else{
        spdlog::warn("no connect but sendMsg conn fd:{}", fd);
    }
}

void TcpConnection::Send(const char* buffer,size_t size) {
    if(isConnected())
    {
        if (loop->isInLoopThread())
        {
            sendInLoop(buffer,size);
        }else
        {
            loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buffer,size));
        }
    }else{
        spdlog::warn("no connect but sendMsg conn fd:{}", fd);
    }
}


void TcpConnection::sendInLoopBuffer(const std::shared_ptr<Buffer>& buffer)
{
    assert(buffer);
    sendInLoop(buffer->begin() + buffer->getReadIndex(), buffer->getReadable());
    buffer->retrieve(buffer->getReadable());
}

void TcpConnection::Send(const std::shared_ptr<Buffer>& buffer) {
    if(isConnected())
    {
        if (loop->isInLoopThread())
        {
//            spdlog::info("loop->isInLoopThread()");
            sendInLoop(buffer->begin() + buffer->getReadIndex(), buffer->getReadable());
            buffer->retrieve(buffer->getReadable());
        }else
        {
//            spdlog::info("loop->runInLoop {}", buffer->getReadable());
            loop->runInLoop(std::bind(&TcpConnection::sendInLoopBuffer,this, buffer));
        }
    }else{
        spdlog::warn("no connect but sendMsg conn fd:{}", fd);
    }
}

void TcpConnection::WriteHandle(const TcpConnection::ptr &ptr) {
    // 事件可写
    size_t n = ::write(fd, outputBuffer.begin() + outputBuffer.getReadIndex(), outputBuffer.getReadable());
    outputBuffer.retrieve(n);
//    spdlog::info("write date: {}", n);
    if (outputBuffer.getReadable() == 0)
    {
        // 说明写完了 ， 取消关注写事件
//        spdlog::info("write date over");
        channel.disableWriting();
        if (writeCompleteCallBack)writeCompleteCallBack(shared_from_this());
    }
}
void TcpConnection::ReadHandel(const TcpConnection::ptr & ptr) {

    auto &buffer = ptr->getInputBuffer();
    ssize_t n = buffer.read(ptr->getFd());
    if (n > 0) {
        if (readCallBack)readCallBack(ptr, buffer, n); // 执行客户回调
    }else if(n == 0)
    {
        CloseHandel(ptr);
    }else{
        ErrorHandel(ptr);
    }

}


void TcpConnection::CloseHandel(const TcpConnection::ptr & ptr) {

    setState(CLOSE);
    ptr->setState(TcpConnection::CLOSE);
    ptr->getChannel()->disableAll();
    ptr->getChannel()->remove();
    ::close(ptr->getFd());
    if (connectCallBack)connectCallBack(ptr); // 处理断开连接
    if (closeCallBack)closeCallBack(ptr);
    SPDLOG_INFO("close fd: {}",ptr->getFd());

}

void TcpConnection::ErrorHandel(const TcpConnection::ptr & ptr) {
    setState(ERROR);
    ptr->setState(TcpConnection::ERROR);
    ptr->getChannel()->disableAll();
    ptr->getChannel()->remove();
    ::close(ptr->getFd());
    if (connectCallBack)connectCallBack(ptr); // 处理断开连接
    if (errorCallBack)errorCallBack(ptr);
}





void TcpConnection::setConnectCallBack(const TcpConnection::ConnectCallBack &callBack) {
    connectCallBack = callBack;
}

void TcpConnection::setReadCallBack(const TcpConnection::ReadCallBack &callBack) {
    readCallBack = callBack;
}

void TcpConnection::setCloseCallBack(const TcpConnection::CloseCallBack &callBack) {
    closeCallBack = callBack;
}



void TcpConnection::setWriteCallBack(const TcpConnection::WriteCallBack &callBack) {
    writeCallBack = callBack;
}

void TcpConnection::setErrorCallBack(const TcpConnection::ErrorCallBack & callBack) {
    errorCallBack = callBack;
}

void TcpConnection::sendInLoop(const char* buffer,size_t size) {
//    spdlog::info("sendInLoop");
    size_t n = ::write(fd, buffer, size);
//    spdlog::info("msg:{}  n:{}",message,n);
    if (n < size)
    {
        // 说明写入不足，需要将剩下的放在buffer里面
        char* p = const_cast<char *>(buffer + n);
        if (outputBuffer.getWriteIndex() + size - n > outputBuffer.getCapacity())
        {
            // 这里getCapacity应该换成用户自定义大小 如果超出自定义大小则执行
            if (highWriteCallBack)highWriteCallBack(shared_from_this());
        }
        outputBuffer.append(p, size - n); // 添加到buffer里面
        channel.setWriteCallBack(std::bind(&TcpConnection::WriteHandle, this, shared_from_this()));
        channel.enableWriting(); // 关注写事件，啥时候能写了，就 写 并且 关闭写事件
    }
}

void TcpConnection::setWriteCompleteCallBack(const TcpConnection::WriteCompleteCallBack & callBack) {
    writeCompleteCallBack = callBack;
}

void TcpConnection::setHighWriteCallBack(const TcpConnection::HighWriteCallBack & callBack) {
    highWriteCallBack = callBack;
}

void TcpConnection::shutdown() {
    loop->runInLoop(std::bind(&TcpConnection::shutdownWriteInLoop, this));
}

void TcpConnection::shutdownWriteInLoop() {
    setState(CLOSE);
    if (channel.isWriting())
    {
        Socket::shutdownWrite(channel.fd());
    }
    channel.disableAll();
    ::close(fd);
    if (connectCallBack)connectCallBack(shared_from_this());
    if (closeCallBack)closeCallBack(shared_from_this());

}

void TcpConnection::setState(TcpConnection::STATE s)   { state = s; }

Buffer &TcpConnection::getInputBuffer(){ return inputBuffer; }