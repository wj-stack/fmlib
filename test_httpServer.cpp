#include <sys/timerfd.h>
#include <csignal>
#include "spdlog/spdlog.h"
#include "fm/EventLoop.h"
#include "fm/TcpServer.h"
#include "fm/HttpServer.h"
#include "fm/TcpConnection.h"
#include "fm/EventLoopThread.h"
#include <iostream>
#include <memory>
#include "jsoncpp/include/json/json.h"
#include "MemoryTools/MemorySearchTools.h"
#include "LexicalCast/LexicalCast.h"

void httpCallBack(RequestHeader &requestHeader, ResponseHeader &responseHeader)
{
    responseHeader.setData("<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "<meta charset=\"utf-8\">\n"
                           "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/base.css\"/>\n"
                           "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/index.css\"/>\n"
                           "  <title>百度一下，你就知道</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "    <div class=\"nav\">\n"
                           "    \t<span id=\"nav-b\">\n"
                           "      <a href=\"\"class=\"nav-a\">新闻</a>\n"
                           "      <a href=\"\"class=\"nav-a\">hao123</a>\n"
                           "      <a href=\"\"class=\"nav-a\">地图</a>\n"
                           "      <a href=\"\"class=\"nav-a\">视频</a>     \n"
                           "      <a href=\"\"class=\"nav-a\">贴吧</a>\n"
                           "      <a href=\"\"class=\"nav-a\">学术</a>\n"
                           "      <a style=\"font-weight:normal\" href=\"\"class=\"nav-a\">登录</a> </span>\n"
                           "      <li id=\"zzr\"><span id=\"nav-b\"><a style=\"font-weight:normal\"  href=\"\" class=\"nav-a\">设置</a> </span>\n"
                           "      <ul>\n"
                           "        <li><a href=\"#\" class=\"shezhi\">搜索设置</a></li>\n"
                           "        <li><a href=\"#\" class=\"shezhi\">高级搜索</a></li>\n"
                           "        <li><a href=\"#\" class=\"shezhi\">关闭预测</a></li>\n"
                           "        <li><a href=\"#\" class=\"shezhi\">搜索历史</a></li>\n"
                           "      </ul></li>\n"
                           "      <a class=\"more\" href=\"\">更多产品</a>\n"
                           "      </div>       \n"
                           "     <div id=\"logo\">\n"
                           "      <img src=\"img/bd_logo1.png\" width=\"270\" height=\"120\">\n"
                           "      </div>     \n"
                           "      <form>  \n"
                           "      \t\t  <input type=\"text\">\n"
                           "    \t<input type=\"submit\" name=\"send\" class=\"search_butt\" value=\"百度一下\" />\n"
                           "    \t     <!--<button type=\"button\">百度一下</button>-->\n"
                           "    </form>\n"
                           "  <div class=\"footer\">\n"
                           "  \t<div class=\"img\"><img src=\"img/zbios_x2_9d645d9.png \"width=\"60\" height=\"60\">\n"
                           "  \t<br /><p><b>百度</b></p>\n"
                           "  \t</div>\n"
                           "  \t<div class=\"about\">\n"
                           "  \t\t<a href=\"\"class=\"about-a\">把百度设为主页</a>\n"
                           "  \t\t<a href=\"\"class=\"about-a\">关于百度</a>\n"
                           "  \t\t<a href=\"\"class=\"about-a\">About Baidu</a>\n"
                           "  \t\t<a href=\"\"class=\"about-a\">百度推广</a>\n"
                           "  \t</div>\n"
                           "  \t<div class=\"foot\">\n"
                           "©2019&nbsp;Baidu&nbsp;\n"
                           "  \t\t<a href=\"\">使用百度前必读</a>\n"
                           "  \t\t<a href=\"\">意见反馈 </a>&nbsp;\n"
                           "  \t\t京ICP证030173号  <img src=\"img/icons_1.png\"/>&nbsp;&nbsp;\n"
                           "  \t\t<a href=\"\">京公网安备11000002000001号</a><img src=\"img/icons_2.png\"/>\n"
                           "  \t</div> \n"
                           "  </div>\n"
                           "    </div>\n"
                           "</body>\n"
                           "</html>\n"
                           "");
}

int main()
{
//    spdlog::set_level(spdlog::level::warn);
    EventLoop eventLoop;
    InetAddress inetAddress(9991, false);
    HttpServer httpServer(&eventLoop, &inetAddress);
    httpServer.setHttpCallBack(httpCallBack);
    httpServer.start();
    eventLoop.loop();
}

