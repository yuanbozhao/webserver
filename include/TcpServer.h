#pragma once
#include <map>
#include <memory>
#include <mutex>

#include "EventLoop.h"
#include "socket.h"
#include "Channel.h"
#include "Accpetor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "noncopyable.h"
// 有多个connection 和一个acceptor,对外接口，需要事件循环去执行不同的channel

class TcpServer : noncopyable
{
private:
  uint16_t buffer_sep_;                              // buffer类型
  std::unique_ptr<EventLoop> mainloop_;              // 一个TcpServer 可以有多个事件循环,该为主事件循环
  std::vector<std::unique_ptr<EventLoop>> subloops_; // 存放从事件循环
  Acceptor acceptor_;                                // 一个tcpserver 只能有一个acceptor
  int threadnum_;                                    // 线程池的大小，即从事件循环的个数
  ThreadPool threadpool_;                            // 线程池
  std::mutex mmutex_;                                // 保护conns_的互斥锁
  // 连接的connection， 管理连接的connection
  std::map<int, spConnection> conns_;                

  // 回调EchoServer::HandleNewConnection()。
  std::function<void(const spConnection &)> newconnectioncb_;
  // 回调EchoServer::HandleClose()。
  std::function<void(const spConnection &)> closeconnectioncb_;
  // 回调EchoServer::HandleError()。
  std::function<void(const spConnection &)> errorconnectioncb_;
  // 回调EchoServer::HandleMessage()。
  std::function<void(const spConnection &, Buffer* message)> onmessagecb_;
  // 回调EchoServer::HandleSendComplete()。
  std::function<void(const spConnection &)> sendcompletecb_;
  // 回调EchoServer::HandleTimeOut()。
  std::function<void(EventLoop *)> timeoutcb_;
  // 回调webserver:: removestate
  std::function<void(int)>removestatcallback_;

public:
  TcpServer(const std::string &ip, const uint16_t port, int threadnum, int buffer_sep);
  ~TcpServer();

  void start(); // 运行事件循环
  // 删除conns_的connection对象，在eventloop::handletimer() 中回调此函数
  void removeconn(int fd); 
  void stop();  // 停止IO线程和事件循环
  // 处理新客户端的请求
  void newconnection(std::unique_ptr<Socket> clientsock);
  // 关闭客户端
  void closeconnection(const spConnection &conn); 
  // 客户端的连接错误                
  void errorconnection(const spConnection &conn); 
  // 处理收到的消息                
  // void onmessage(const spConnection &conn, std::string &message); 
  // // 数据发送完成过后回调
  //void sendcomplate(const spConnection &conn);    
  // epoll_wait()超时，在eventloop类中回调此函数                
  void epolltimeout(EventLoop *loop);

  void setnewconnectioncb(const std::function<void(const spConnection &)> &fn);
  void setcloseconnectioncb(const std::function<void(const spConnection &)> &fn);
  void seterrorconnectioncb(const std::function<void(const spConnection &)> &fn);
  void setonmessagecb(const std::function<void(const spConnection &, Buffer* message)> &fn);
  void setsendcompletecb(const std::function<void(const spConnection &)> &fn);
  void settimeoutcb(const std::function<void(EventLoop *)> &fn);
  void setremovestatecb(const std::function<void(int)>& fn);
};