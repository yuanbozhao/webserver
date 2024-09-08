#pragma once

#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <queue>
#include <mutex> 
#include <atomic>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include  <map>

#include "Connection.h"
#include "poller.h"
#include "Timestamp.h"
// 事件循环类
// 包含了 channel 和 epoll
// 一个eventloop 对应 一个epoll 
// 一个eventloop 对应 多个channel
class Channel;
class Poller;
class Connection;
using spConnection = std::shared_ptr<Connection>;

class EventLoop : noncopyable
{
private:
  int timetvl_;       // 闹钟时间间隔， 单位：秒
  int timeout_;       // connection对象超时的时间：单位：秒
  std::unique_ptr<Poller> poller_;     // 每个事件循环只有一个epoll
  std::function<void(EventLoop*)> epolltimeoutcallback_;
  pid_t threadid_;                // 事件循环所在线程的id

  std::queue<std::function<void()>> taskqueue_; // 用于IO线程
  std::mutex mutex_;              // 创建锁

  int wakeupfd_;                  // 用于唤醒事件循环线程的eventfd;
  // 当mainloop获取一个新用户的channel,通过轮寻算法选择subloop
  std::unique_ptr<Channel> wakechannel_; // eventfd的channel
  int timerfd_;         // 定时器的fd
  std::unique_ptr<Channel> timerchannel_; // 定时器的channel
  bool mainloop_;       // true-是主事件循环，false是从事件循环
  std::mutex mmutex_;   // 保护conns_的互斥锁
  std::map<int,spConnection> conns_;
  // 1、 在事件循环中增加map<int, spConnect> conns_容器，存放运行在该事件循环上的全部的connection对象 
  // 2、 如果闹钟时间到了，遍历conns_, 判断每个connection对喜爱嗯是否超时
  // 3、 如果超时，从conns_ 中删除connection对象
  // 4、 从tcpserver.conns_中删除connection 对象
  Timestamp pollReturnTime_;      // poller返回发生事件的channels的时间点
  std::function<void(int)> timercallback_; 
  // 删除tcpserver中的connecton对象

  using ChannelList = std::vector<Channel*>;
  ChannelList activeChannels_;   // 一个事件循环包含的channel
  std::atomic_bool stop_; // 初始值为false,设为true ，表示停止事件循环
public:

  EventLoop(bool mainloop, int timetvl= 30, int timeout = 50);  // 在构造函数中创建epoll对象ep_
  ~EventLoop();
  //Epoll* ep();
  void run();                                   // 事件循环代码
  void stop();                                  //
  void updatechannel(Channel *ch);              // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
  void removechannel(Channel *ch);           // 从红黑树中删除channel
  bool isinloopthread() const;               // 判断当前线程是否是事件循环线程
  void queueinloop(std::function<void()> fn);   // 把任务添加到队列中
  void setepolltimeoutcallback(std::function<void(EventLoop*)>fn);
  // 设置epoll_wait()超时的回调函数
  void wakeup();                                // 用eventfd唤醒事件循环
  void handlewakeup();                          // 事件循环唤醒执行的函数
  void handletimer();                           // 闹钟响执行的函数
  void newconnection(spConnection conn);        // 向map容器中加入connection
  void settimercallback(std::function<void(int)>fn);  // 时间超时机制
  //判断当前的eventloop对象是否在自己的线程里面
  void erase(int fd)
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_.erase(fd);
  }
};