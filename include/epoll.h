#pragma once

#include<stdio.h>
#include<errno.h>
#include<strings.h>
#include<string.h>
#include<sys/epoll.h>
#include<unistd.h>   // close

#include "poller.h"

class Channel;      // 申明channel类
class EventLoop;
// epoll: 负责更新红黑树，epoll事件由fd改为ptr(channel)，
// 对已经发生的事件使用channel接收，并返回channel

class Epoll : public Poller
{
private:
  //static const int MaxEvents = 1000;     // epoll_wait() 返回事件数组的大小
  //epoll_event events_[MaxEvents];       // 存放poll_wait() 返回事件的数组
  
  int epollfd_ = -1;              // epoll 句柄，在构造函数中创建
  static const int KInitEventListSize = 16;
  using EventList = std::vector<epoll_event>;
  EventList events_;              // 存放poll_wait() 返回事件的数组,可以扩容 

  // 填写活跃的连接
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
public:
  // 在构造函数中创建了epollfd
  Epoll(EventLoop* loop);   
  ~Epoll() override;                   // 虚析构
  Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
  
  // 把channel添加/更新到红黑树上，channel中有fd,也需要监视的事件
  void undatechannel(Channel* ch) override;   
  //void addfd(int fd,uint32_t op);   // 把FD和需要监视的事件添加到红黑树中
  void removechannel(Channel* ch) override;    // 从红黑树中删除channel
  void updata(int option, Channel* ch);
  //ChannelList loop(int timeout =-1);
  // std::vector<epoll_event> loop(int timeout =-1);
  // 运行epoll_wait(), 等待事件的发生，已发生的事件用vector容器返回
};