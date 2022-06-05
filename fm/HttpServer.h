//
// Created by Administrator on 2022/4/27.
//

#ifndef FM_HTTPSERVER_H
#define FM_HTTPSERVER_H

#include "TcpServer.h"
#include "InetAddress.h"
#include <memory>
#include <functional>
#include <sstream>
#include "spdlog/spdlog.h"

class RequestHeader{
public:
    void parse(const std::string& s);

    std::string &getFunc();

    std::string &getPath();

    std::string &getProtocol();

    std::string &getData();

    std::map<std::string,std::string> &getHeader();

    std::string toString();

    std::map<std::string,std::string>& getParam();

    std::string getParam(const std::string& key,const std::string& defaultValue);


private:
    std::map<std::string,std::string> header_;
    std::map<std::string,std::string> param_;
    std::string func_;
    std::string path_;
    std::string protocol_;
    std::string data_;
};


class ResponseHeader{
public:
    ResponseHeader();

    std::map<std::string,std::string> &getHeader();

    void setHeader(const std::string& name,const std::string& val);

    std::string toString();

public:
    void setState(const std::string &state);

    void setStateName(const std::string &s);

    void setProtocol(const std::string &protocol);

    void setData(const std::string& data);


private:
    std::map<std::string,std::string> header_;
    std::string stateName_;
    std::string state_;
    std::string protocol_;
    std::string data_;
};


class HttpServer {
public:

    typedef std::function<void(RequestHeader &, ResponseHeader &)> HttpCallBack;

    void connectCallBack(const TcpConnection::ptr & conn);

    void readCallBack(const TcpConnection::ptr& conn,Buffer& buffer , int len);

    void errorCallBack(const TcpConnection::ptr& conn);

    void closeCallBack(const TcpConnection::ptr& conn);

    HttpServer(EventLoop* loop,InetAddress* address) ;

    void start();

    void setHttpCallBack(const HttpCallBack& callBack);

    void Get(const std::string& ,const HttpCallBack&);

    void Post(const std::string& ,const HttpCallBack&);

private:
    EventLoop* eventLoop;
    InetAddress *inetAddress;
    TcpServer tcpServer;
    HttpCallBack httpCallBack;
    std::map<std::string,HttpCallBack> GetCallBack;
    std::map<std::string,HttpCallBack> PostCallBack;
};


#endif //FM_HTTPSERVER_H
