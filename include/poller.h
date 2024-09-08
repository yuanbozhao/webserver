#pragma once
#include <vector>
#include <unistd.h>
#include <iostream>
#include <unordered_map>

#include "noncopyable.h"
#include "Channel.h"
#include "Timestamp.h"
#include "Logger.h"

class Channel;      // 申明channel类
class EventLoop;
// poller 基类： 

class Poller : noncopyable
{
private:
  EventLoop* ownerLoop_;                // 定义epoll所属的事件循环   
protected:
  using ChannelMap = std::unordered_map<int, Channel*>;
  ChannelMap channels_;
public:
  using ChannelList = std::vector<Channel*>;
  // 在构造函数中创建了epollfd
  Poller(EventLoop* loop);   
  virtual ~Poller() = default;                 // 虚析构->继承
  // 给所有的IO复用保留统一的接口
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
  
  // 把channel添加/更新到IO复用
  virtual void undatechannel(Channel* ch) =0;   
  //void addfd(int fd,uint32_t op);   // 把FD和需要监视的事件添加到红黑树中
  virtual void removechannel(Channel* ch)=0;    // 从红黑树中删除channel

  bool hasChannel(Channel* channel) const;  // 判断channel是否在poller中
  
  // 通过该接口获取具体的IO复用实现对象
  static Poller* newDefaultPoller(EventLoop* loop);
};