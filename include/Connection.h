#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <any>
#include "socket.h"
#include "inetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "noncopyable.h"
#include "Logger.h"

class EventLoop;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;

class Connection : public noncopyable, public std::enable_shared_from_this<Connection>
{
private:
  EventLoop *loop_;                    // Acceptor对应的事件循环
  std::unique_ptr<Socket> clientsock_; // 改为独享指针，不需要自己析构
  //Socket *clientsock_;
  std::unique_ptr<Channel> clientchannel_;
  // 查看
  uint16_t buffer_sep_;     // buffer种类
  Buffer inputbuffer_;
  // 发送缓冲区
  Buffer outbuffer_;

  // 接收缓冲区
  std::atomic_bool disconnect_;                                        // 原子类型，客户端连接是否断开，若断开，设置为true
  std::function<void(const spConnection&)> closecallback_;                    //关闭fd的回调
  std::function<void(const spConnection&)> errorcallback_;                    //出错
  std::function<void(const spConnection&, Buffer*)> onmessagecallback_; // 处理收到数据
  std::function<void(const spConnection&)> sendcomplate_;                     // 数据发送成功
  Timestamp lasttime_;                                                 // 事件戳，创建conection的事件戳
  std::mutex mutex_;
  std::any context_;
public:
  Connection(EventLoop *loop, std::unique_ptr<Socket> clientsocket,int buffer_sep);
  ~Connection();

  int fd() const;         //  返回fd成员
  std::string ip() const; // 返回ip地址
  uint16_t port() const;  // 返回port
  bool connectEstablished();  // 连接建立
  void closecallback(); // TCP连接关闭（断开）的回调函数，供channel回调
  void errorcallback(); // TCP连接错误的回调函数，供channel回调
  void onmessage();     // 处理收到的数据
  void writecallback(); // 写事件

  void setclosecallback(const std::function<void(const spConnection&)>& fn);
  void seterrorcallback(const std::function<void(const spConnection&)> &fn);
  void setonmessagecallback(const std::function<void(const spConnection&, Buffer *)>& fn);
  void setsendcomplatecallback(const std::function<void(const spConnection&)>& fn);

  // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。
  
  void sendinloop(const char *data, size_t size);
  // 发送数据，不管在任何线程中，都是调用此函数发送数据。
  void send(const char *data, size_t size);
  bool timeout(time_t now, int val); // 判断TCP连接是否超时，空闲太久

  void setContext(const std::any& context)
  { context_ = context; }

  const std::any& getContext() const
  { return context_; }

  std::any* getMutableContext()
  { return &context_; }
};