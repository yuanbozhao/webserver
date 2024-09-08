#include "EventLoop.h"

// 防止一个线程创建多个eventloop
// __thread EventLoop* t_loopInThisThread = nullptr;
// 本项目的思路反而是在一个mainloop中使用 vector 创建多个subloop,并将其绑定到threadpool的 addtask中

// // 创建定时器fd
int createtimerfd(int sec = 30)
{
  int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
  // 创建timerfd。
  struct itimerspec timeout;
  // 定时时间的数据结构。
  memset(&timeout, 0, sizeof(struct itimerspec));
  timeout.it_value.tv_sec = sec; // 定时时间，固定为5，方便测试。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd, 0, &timeout, 0);
  return tfd;
}

EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
    : poller_(Poller::newDefaultPoller(this)), mainloop_(mainloop),
      timetvl_(timetvl), timeout_(timeout),
      wakeupfd_(eventfd(0, EFD_NONBLOCK)),
      wakechannel_(new Channel(this, wakeupfd_)),
      timerfd_(createtimerfd(timeout_)), timerchannel_(new Channel(this, timerfd_)), stop_(false)
{

  LOG_DEBUG("EventLoop %p created %l in this thread %d \n", this,syscall(SYS_gettid));

  // if (t_loopInThisThread)
  // {
  //    LOG_FATAL("Anathor EventLoop %p exists in this thread %d \n",t_loopInThisThread, syscall(SYS_gettid));
  // }
  // else{
  //   t_loopInThisThread = this;
  // }
  
  // std::cout << "wakechannel_: "<<wakechannel_->fd() <<std::endl;
  // std::cout << "timerchannel_: "<<timerchannel_->fd() <<std::endl;
  wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup, this));
  wakechannel_->enablereading();
  timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer, this));
  timerchannel_->enablereading();
}

EventLoop::~EventLoop()
{
  wakechannel_->disableall();
  wakechannel_->remove();
  ::close(wakeupfd_);
  timerchannel_->disableall();
  timerchannel_->remove();
  ::close(timerfd_);
  // t_loopInThisThread = nullptr;
  //delete ep_;
}

void EventLoop::run()
{
  // 获取线程id -》必须放在运行中  ??? 
  // 因为subloop是在 mainloop中创建，在 构造函数中获取的线程不是子线程
  this->threadid_ = syscall(SYS_gettid);
  //LOG_INFO("EventLoop %p start looping \n", this);
  while (stop_ == false)
  {
    activeChannels_.clear();
    pollReturnTime_=  poller_->poll(-1,&activeChannels_);
    if (activeChannels_.size() == 0)
    {
      // 回调tcpserver::epolltimeout()
      this->epolltimeoutcallback_(this);
    }
    else
    {
      // poller 监听到哪些channel发生事件，然后上报给eventloop,处理事件
      for (auto &ch : activeChannels_)
      {
        ch->handleevent(pollReturnTime_); // 处理epoll_wait()返回事件
      }
    }
  }
}

// Epoll *EventLoop::ep()
// {
//   return ep_;
// }

// 停止事件循环
// 1. loop在自己线程调用退出
// 2. 在非loop的线程中调用loop的stop
void EventLoop::stop()
{
  stop_ = true;
  if(!isinloopthread())
    wakeup(); 
  // 唤醒事件循环，如果没有该代码，事件还在下次闹钟相是或者epoll_wait()超时才会停止。
}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop *)> fn)
{
  this->epolltimeoutcallback_ = fn;
}

void EventLoop::updatechannel(Channel *ch)
// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
{
  this->poller_->undatechannel(ch);
}

void EventLoop::removechannel(Channel *ch) // 从事件循环中删除channel
{
  //ep_->removechannel(ch);
  poller_->removechannel(ch);
}

// 判断当前线程是否是事件循环线程(I/O线程)
bool EventLoop::isinloopthread() const
{
  return this->threadid_ == syscall(SYS_gettid);
}

// 把任务添加到队列中,回调:Connection::sendinloop
void EventLoop::queueinloop(std::function<void()> fn)
{
  {
    std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁
    taskqueue_.emplace(fn);
    if(!isinloopthread()) wakeup(); // 唤醒相应的loop的线程事件循环
  }
  
}

// 用eventfd唤醒事件循环
// 用来唤醒loop所在的线程 向wakeupfd写入一个数据，wakeupchannel就发生写事件，当前loop线程就会被唤醒
void EventLoop::wakeup() 
{
  //std::cout << " ---------------EventLoop::wakeup(--"<< std::endl;
  uint64_t val = 1;
  ssize_t n = write(wakeupfd_, &val, sizeof(val));
  if (n != sizeof(val))
  {
    LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
  }
}

// 事件循环唤醒执行的函数
void EventLoop::handlewakeup()
{
  //std::cout << "handlewakeup() thread id is: " << syscall(SYS_gettid) << std::endl;

  uint64_t val;
  ssize_t n = read(wakeupfd_, &val, sizeof(val));
  // 从eventfd中读取是数据，如果不读取，eventfd的读事件会一直触发
  if (n != sizeof(val))
  {
    LOG_ERROR("EventLoop::wakeup() read %lu bytes instead of 8 \n", n);
  }

  std::function<void()> fn;
  std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁
  while (taskqueue_.size() > 0)
  {
    fn = std::move(taskqueue_.front());
    taskqueue_.pop(); // 出队一个任务
    fn();             // 执行任务
  }
}

// 闹钟响
void EventLoop::handletimer() // 执行的函数
{
  struct itimerspec timeout;
  // 定时时间的数据结构。
  memset(&timeout, 0, sizeof(struct itimerspec));
  timeout.it_value.tv_sec = timetvl_; // 定时时间，固定为5，方便测试。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(timerfd_, 0, &timeout, 0);

  if (mainloop_)
  {
    //printf("主事件循环 is===== %d .fd ======", syscall(SYS_gettid));
  }
  else
  {
    time_t now = time(0); // 获取当前时间
    //LOG_INFO("获取当前时间");
    for (auto aa = conns_.begin(); aa!= conns_.end();)
    {
      //std::cout << aa->first << " ";
      if (aa->second->timeout(now, timeout_))   // 大于20s就超时了
      {
        //LOG_INFO("从事件循环的闹钟事件到了.\n");
        auto temp = aa;
        {
          std::lock_guard<std::mutex> gd(mmutex_);
          aa = conns_.erase(temp);
        }
        timercallback_(temp->first);
      }
      else
      {
        aa++;
      }
    }
    //std::cout << std::endl;
  }
}

// 向map容器中加入 connection
void EventLoop::newconnection(spConnection conn)
{
  std::lock_guard<std::mutex> gd(mmutex_);
  conns_[conn->fd()] = conn;
}

void EventLoop::settimercallback(std::function<void(int)> fn)
{
  timercallback_ = fn;
}