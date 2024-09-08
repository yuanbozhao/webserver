#pragma once
#include <memory>
#include <functional>

#include "socket.h"
#include "inetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Acceptor : noncopyable
{
private:
  EventLoop* loop_;
  //const std::unique_ptr<EventLoop>& loop_;        // Acceptor对应的事件循环
  Socket servsock_;         // 服务端用于监听的socket，在构造函数中创建
  Channel acceptchannel_;   // acceptor对应的channel,在构造函数中创建
  std::function<void(std::unique_ptr<Socket>)> newconnectioncb_;  // 处理新客户端连接请求的回调函数，指向tcpserver::newconnection()
public:
  Acceptor(EventLoop* loop,const std::string &ip,const uint16_t port);
  ~Acceptor(); 
  void newconnection();           // 处理新客户端的请求 
  void setnewconnectioncb(const std::function<void( std::unique_ptr<Socket>)>&fn);
};