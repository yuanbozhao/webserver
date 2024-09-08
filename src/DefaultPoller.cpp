#include "poller.h"
#include "epoll.h"

#include <stdlib.h>

// 用于实现 poller对具体接口的实现-》感觉类似回调的思路
Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if(::getenv("MUDUO_USE_POLL")){
    return nullptr;     // poll
  }
  else{
    return new Epoll(loop);  // epoll
  }
}