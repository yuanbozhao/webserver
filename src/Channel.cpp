#include "Channel.h"

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), tied_(false),events_ (0),revents_ (0),inepoll_(false)
{
}

Channel::~Channel()
{
  //std::cout << "---析构造" << std::endl;
  // 不需要销毁ep，也不能关闭fd,因为这两个东西不属于channel类，channel类只是需要他们
}

// 防止channel被手动remove掉
// 何时调用？
void Channel::tie(const std::shared_ptr<void> &obj)
{
  /**
     * @brief channel的tie方法什么时候调用？一个TcpConnection新连接创建的时候，
     * TcpConnection => Channel;
  */
  tie_ = obj;
  tied_ = true;
}

int Channel::fd() // 返回 fd_成员
{
  return fd_;
}
void Channel::useet() // 采用边缘触发
{
  events_ |= EPOLLET;
}
void Channel::enablereading() // 让epoll_wait()监视fd的读事件
{
  events_ |= EPOLLIN;
  //LOG_INFO("%s-> fd=%d events=%d index=%d ,EPOLLIN\n",__FUNCTION__,this->fd(), this->events(), this->inpoll());
  loop_->updatechannel(this);
}
void Channel::disablereading() // 让epoll_wait()关闭fd的读事件，关闭读事件
{
  events_ &= ~EPOLLIN;
  //LOG_INFO("%s-> fd=%d events=%d index=%d ,~EPOLLIN\n",__FUNCTION__,this->fd(), this->events(), this->inpoll());
  loop_->updatechannel(this);
}
void Channel::enablewriting() // 让epoll_wait()监视fd的写事件，注册读事件
{
  //LOG_INFO("%s-> fd=%d events=%d index=%d\n",__FUNCTION__,this->fd(), this->events(), this->inpoll());
  events_ |= EPOLLOUT;
  loop_->updatechannel(this);
}
void Channel::disablewriting() // 让epoll_wait()关闭fd的写事件，关闭读事件
{
  
  //LOG_INFO("%s-> fd=%d events=%d index=%d ,EPOLLIN\n",__FUNCTION__,this->fd(), this->events(), this->inpoll());
  events_ &= ~EPOLLOUT;
  loop_->updatechannel(this);
}

void Channel::setinepoll() // 把inepoll 设置为true
{
  inepoll_ = true;
}
void Channel::setrevents(uint32_t ev) // 设置revents成员的值为ev
{
  this->revents_ = ev;
}
bool Channel::inpoll() // 判断该事件是否注册
{
  return inepoll_;
}
uint32_t Channel::events() // 返回注册事件
{
  return events_;
}
uint32_t Channel::revents() // 返回发生事件
{
  return revents_;
}

// 处理事件
void Channel::handleevent(Timestamp receiveTime)
{
  if (tied_)
  {
    std::shared_ptr<void> guard = tie_.lock(); // 弱指针提升为强指针
    if (guard)
    {
      this->handleEventWithGuard(receiveTime);
    }
  }
  else
  {
    this->handleEventWithGuard(receiveTime);
  }
}

// 处理保护事件
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
  //LOG_INFO("channel handleEvent revents:%d\n", revents());

  if (revents_ & EPOLLRDHUP) // 对方关闭
  {
    if (closecallback_)
    {
      //std::cout << "--------EPOLLRDHUP" << std::endl;
      remove();
      this->closecallback_();
    }
    // std::cout << "client [" << fd() << "]"
    //           << "disconnected" << std::endl;
    // close(fd());
  }
  else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区有数据可读
  {
    // if (this->islisten_) // 如果是listenfd有事件，表示有新的客户端
    // {
    //   newconnection(servsock);
    // }
    // 读事件的回调函数
    //std::cout << "EPOLLIN | EPOLLPRI :" << this->fd()<<" Thread: "<<syscall(SYS_gettid) <<std::endl;
    if (readcallback_)
      readcallback_(receiveTime);
    //如果是acceptchannel，将回调Acceptor::newconnection()，如果是clientchannel，将回调Connection::onmessage()。

    // else // 通信的socket 有读事件
    // // 分三种情况：1、client close() 2、client recv() 3 读到事件
    // {
    //   onmessage();
    // }
  }
  else if (revents_ & EPOLLOUT) // 有数据需要写，暂时没代码
  {
    //std::cout << "EPOLLOUT" << std::endl;
    if(writecallback_)
        writecallback_();
    // 只有connection才有写事件将回调Connection::setwritecallback()
  }
  else // 其他事件
  {
    remove(); // 从事件循环中删除channel
    //std::cout << "error" << std::endl;
    if(errorcloseback_)
    this->errorcloseback_();
    // std::cout << "client[" << fd_ << "] error" << std::endl;
    // close(fd_); // 关闭客户fd
  }
}

// 取消全部事件
void Channel::disableall()
{
  events_ = 0;
  loop_->updatechannel(this);
}

// 从事件循环中删除channel
void Channel::remove()
{
  disableall();               // 取消事件
  loop_->removechannel(this); // 从红黑树中删除fd
}

EventLoop *Channel::ownerLoop() // 返回channel所属的eventloop;
{
  return loop_;
}
// 设置fd读事件的回调函数
void Channel::setreadcallback(std::function<void(Timestamp receiveTime)> fn)
{
  this->readcallback_ = std::move(fn);
}

void Channel::setwritecallback(std::function<void()> fn)
{
  this->writecallback_ = std::move(fn);
}

void Channel::setclosecallback(std::function<void()> fn)
{
  this->closecallback_ = std::move(fn);
}

void Channel::seterrorcallback(std::function<void()> fn)
{
  this->errorcloseback_ = std::move(fn);
}