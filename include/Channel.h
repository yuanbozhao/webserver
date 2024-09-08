#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>

#include "EventLoop.h"
#include "inetAddress.h"
#include "socket.h"
#include "Timestamp.h"
#include "Logger.h"
class EventLoop;

// 一个channel对应一个fd，表示一个红黑树上的事件
// 多个channel对应一个eventloop
// channel负责处理不同的事件，并将不同的事件同步到epoll中

class Channel : public noncopyable
{
private:
  // Channel只对应一个fd，channel和fd是一对一的关系
  int fd_ = -1;

  //Epoll *ep_ = nullptr;   // channel 对应的红黑树，channel与epoll是多对一的关系

  EventLoop *loop_; // Channel对应的事件循环，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop。
  //const std::unique_ptr<EventLoop>& loop_;
  bool inepoll_ = false; // channel是否添加到红黑树上
  uint32_t events_;  // fd_需要监视的事件，listenfd和clientfd需要监视epollin,clientfd 还需要监视epollout
  uint32_t revents_; // fd_已发生的事件

  // channel通道可以获取fd最终发生的具体时间revents_,因此负责回调具体事件
  std::function<void(Timestamp receiveTime)> readcallback_;  // 读事件的回调函数
  std::function<void()> writecallback_; // 写事件的回调函数
  std::function<void()> closecallback_; // 关闭事件的回调函数
  std::function<void()> errorcloseback_; // 出错事件的回调函数

  std::weak_ptr<void> tie_; //这个tie_ 绑定了....
  bool tied_;

  // std::cout << sizeof(Channel) << std::endl;           160
  // std::cout << offsetof(Channel,fd_) <<std::endl;      0       4
  // std::cout << offsetof(Channel,loop_) <<std::endl;    8       8
  // std::cout << offsetof(Channel,inepoll_) <<std::endl; 16      1
  // std::cout << offsetof (Channel,events_) <<std::endl;  20      4
  // std::cout << offsetof(Channel,revents_) <<std::endl; 24      4
  // std::cout << offsetof(Channel,readcallback_) <<std::endl;32  32
  // std::cout << offsetof(Channel,writecallback_) <<std::endl;64 32
  // std::cout << offsetof(Channel,closecallback_) <<std::endl;96 32
  // std::cout << offsetof(Channel,errorclaaback_) <<std::endl;128  32
  // std::cout << sizeof(std::function<void()>) << std::endl; 160

public:
  Channel(EventLoop *loop, int fd);
  //Channel(const std::unique_ptr<EventLoop>& loop, int fd);
  int fd();                     // 返回 fd_成员
  void useet();                 // 采用边缘触发
  void enablereading();         // 让epoll_wait()监视fd的读事件，注册读事件
  void disablereading();        // 让epoll_wait()关闭fd的读事件，关闭读事件
  void enablewriting();         // 让epoll_wait()监视fd的写事件，注册读事件
  void disablewriting();        // 让epoll_wait()关闭fd的写事件，关闭读事件
  void disableall();            // 取消全部事件
  void remove();                // 从事件循环中删除channel
  void setinepoll();            // 把inepoll 设置为true
  void setrevents(uint32_t ev); // 设置revents成员的值为ev
  bool inpoll();                // 返回inepoll成员
  uint32_t events();            // 返回events成员
  uint32_t revents();           // 返回revents成员
  EventLoop *ownerLoop();       // 返回channel所属的eventloop;
  //防止当channel被手动remove掉，channel还正在执行回调操作
  void tie(const std::shared_ptr<void> &obj);

  // 事件处理函数，epoll
  void handleevent(Timestamp receiveTime);
  // 处理保护事件函数
  void handleEventWithGuard(Timestamp receiveTime);

  //void newconnection(Socket *servsock);
  // 处理新客户端的请求

  //void onmessage();
  // 处理对端发过来的消息

  // 处理读事件回调函数
  void setreadcallback(std::function<void(Timestamp receiveTime)> fn);

  // 处理写事件回调函数
  void setwritecallback(std::function<void()> fn);

  // 关闭fd的回调函数，将回调connection::closeback();
  void setclosecallback(std::function<void()> fn);
  // fd发生出错，将回调connection::errorcallback
  void seterrorcallback(std::function<void()> fn);

  ~Channel();
};