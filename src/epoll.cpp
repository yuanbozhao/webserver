#include "epoll.h"

Epoll::Epoll(EventLoop *loop)
    : Poller(loop),
      events_(KInitEventListSize)
{
  this->epollfd_ = epoll_create(1);
  if (this->epollfd_ == -1)
  {
    LOG_FATAL("epoll create() failed:%d \n", errno);
  }
}

Epoll::~Epoll()
{
  ::close(epollfd_);
}

void Epoll::updata(int option, Channel *ch)
{
  //这里主要就是根据operation: EPOLL_CTL_ADD MOD DEL来具体的调用epoll_ctl更改这个channel对应的fd在epoll上的监听事件
  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = ch->events(); //events()函数返回fd感兴趣的事件。
  event.data.ptr = ch;         //这个epoll_event.data.ptr是给用户使用的，
  //epoll不关心里面的内容。用户通过这个可以附带一些自定义消息。这个 epoll_data 会随着
  // epoll_wait 返回的 epoll_event 一并返回。那附带的自定义消息，就是ptr指向这个自定义消息。
  int fd = ch->fd();
  if (::epoll_ctl(epollfd_, option, fd, &event) < 0)
  {
    if (option == EPOLL_CTL_MOD)
    {
      LOG_FATAL("epoll_ctl mod error:%d\n", errno);
    }
    else if (option == EPOLL_CTL_DEL)
    {
      LOG_ERROR("epoll_ctl del error:%d\n", errno);
    }
    else
    {
      LOG_FATAL("epoll_ctl add error:%d\n", errno);
    }
  }
}

// 使用红黑树监听socket事件
void Epoll::undatechannel(Channel *ch)
{
  //LOG_INFO("%s fd=%d events=%d index=%d \n",__FUNCTION__,ch->fd(), ch->events(), ch->inpoll());
  if (ch->inpoll()) // 如果channel在树上，更新
  {
    //LOG_INFO("发送完成");
    updata(EPOLL_CTL_MOD, ch);
  }
  else // 如果channel不再树上
  {
    ch->setinepoll(); // 更新该事件到红黑树
    updata(EPOLL_CTL_ADD, ch);
  }
}

void Epoll::removechannel(Channel *ch)
{
  //LOG_INFO("func = %s => fd=%d \n", __FUNCTION__, ch->fd());
  if (ch->inpoll())
  {
    updata(EPOLL_CTL_DEL, ch);
  }
}

Timestamp Epoll::poll(int timeoutMs, ChannelList *activeChannels)
{
  //LOG_INFO("func=%s => fd total count:%d\n", __FUNCTION__, channels_.size());
  //std::cout << syscall(SYS_gettid)<< "  events_.size(): "<< events_.size()<< std::endl;
  //std::cout << "等待中-------" <<std::endl;
  int infds = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
  int saveError = errno;
  Timestamp now(Timestamp::now());

  // 返回失败
  if (infds < 0)
  {
    if (saveError != EINTR)
    {
      errno = saveError;
      LOG_FATAL("epoll_wait() failed \n");
    }
    // perror("epoll_wait() failed");
    // exit(-1);
  }

  // 超时
  if (infds == 0)
  {
    LOG_DEBUG("%s epoll_wait() timeout\n", __FUNCTION__);
  }
  // 如果infds >0 ， 表示有事件发生的fd的数量
  //LOG_INFO("%d events happend \n", infds);
  fillActiveChannels(infds, activeChannels);
  if (infds == events_.size())
  {
    events_.resize(events_.size() * 2);
  }
  return now;
}

void Epoll::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
  for (int ii = 0; ii < numEvents; ii++)
  {
    Channel *ch = static_cast<Channel *>(events_[ii].data.ptr); // 取出已发生事件的channel
    ch->setrevents(events_[ii].events);                         // 设置channel的revents成员
    activeChannels->emplace_back(ch);
  }
}

// 运行epoll_wait(), 等待事件的发生，已发生的事件用vector容器返回
// std::vector<Channel *> Epoll::loop(int timeout)
// {
//   std::vector<Channel *> channels; // 存放epoll_wait() 返回的事件

//   //bzero(events_, sizeof(events_));
//   int infds = epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeout);
//   // 返回失败
//   if (infds < 0)
//   {
//     perror("epoll_wait() failed");
//     exit(-1);
//   }

//   // 超时
//   if (infds == 0)
//   {
//     std::cout << "epoll_wait() timeout" << std::endl;
//     return channels;
//   }
//   // 如果infds >0 ， 表示有事件发生的fd的数量
//   for (int ii = 0; ii < infds; ii++)
//   {
//     Channel *ch = (Channel *)events_[ii].data.ptr; // 取出已发生事件的channel
//     ch->setrevents(events_[ii].events);            // 设置channel的revents成员
//     channels.emplace_back(ch);
//   }
//   return channels;
// }

/*
void Epoll::addfd(int fd, uint32_t op) // 把FD和需要监视的事件添加到红黑树中
{
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = op;
  if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
  {
    std::cout << "epoll_ctl() failed" << errno << std::endl;
    exit(-1);
  }
}
 // 运行epoll_wait(), 等待事件的发生，已发生的事件用vector容器返回
std::vector<epoll_event> Epoll::loop(int timeout)
{
  std::vector<epoll_event> evs;
  bzero(events_, sizeof(events_));
  int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
  // 返回失败
  if (infds < 0)
  {
    perror("epoll_wait() failed");
    exit(-1);
  }

  // 超时
  if (infds == 0)
  {
    std::cout << "epoll_wait() timeout" << std::endl;
    return evs;
  }
  // 如果infds >0 ， 表示有事件发生的fd的数量
  for (int ii = 0; ii < infds; ii++){
    evs.emplace_back(events_[ii]);
  } 

  return evs;
}
*/
