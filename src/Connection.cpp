#include "Connection.h"

Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock, int buffer_sep)
    : loop_(loop), clientsock_(std::move(clientsock)),
      disconnect_(false), buffer_sep_(buffer_sep),
      inputbuffer_(buffer_sep_), outbuffer_(buffer_sep_),
      clientchannel_(new Channel(loop_, clientsock_->fd()))
{
  //std::cout << "Connection::Connection()" <<std::endl;
  // 将新客户端连接准备读事件，并添加到epoll
  //clientchannel_ = new Channel(loop, clientsock_->fd());
  //LOG_INFO("new connection(fd= %d,ip=%s,port=%d) ok.\n", this->fd(), this->ip().c_str(), this->port());
  // 绑定关闭事件
  clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
  // 绑定出错事件
  clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));

  //connectEstablished();

  //ep.addfd(clientsock->fd(),EPOLLIN | EPOLLET);

  //ev.data.fd = clientsock->fd();
  //ev.events = EPOLLIN | EPOLLET;                    // 采用边缘触发
  //epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev); // 加入事件 红黑树
}

bool Connection::connectEstablished() // 连接建立
{
  clientchannel_->tie(shared_from_this());
  clientchannel_->useet(); // 采用边缘触发
  // 绑定写事件
  clientchannel_->setwritecallback(std::bind(&Connection::writecallback, this));
  // 绑定读事件
  clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
  clientchannel_->enablereading();
  return true;
}

int Connection::fd() const //  返回fd成员
{
  return clientsock_->fd();
}
std::string Connection::ip() const
{
  return clientsock_->ip();
}
uint16_t Connection::port() const // 返回port
{
  return clientsock_->port();
}

// 处理对端发来的数据
void Connection::onmessage()
{
  // if(!this->loop_->isinloopthread()){
  //   clientchannel_->remove(); // 从事件循化中删除channel
  //   closecallback();
  //   LOG_FATAL("不在自己的线程中");
  // }
  // else{
  //   std::cout<< "在自己线程:"<< syscall(SYS_gettid) << std::endl;
  // }
  //std::cout << "Connection :onmessage()---------" <<std::endl;
  char buffer[1024];
  while (true)
  {
    bzero(&buffer, sizeof(buffer));
    ssize_t nread = read(this->fd(), buffer, sizeof(buffer));
    if (nread > 0)
    {
      //send(fd(), buffer, sizeof(buffer), 0);
      inputbuffer_.append(buffer, nread); // 读取的数据
    }
    else if (nread == -1 && errno == EINTR) // 读取数据时信号中断，继续读取
    {
      continue;
    }
    else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    { // 全部数据读取完毕
      std::string message;
      // 当客户端发送大量内容时，服务端是粘在一块的，因此需要处理粘包问题
      while (true)
      {
        // 说明接收缓冲区的内容不完美，退出，否则处理接收到的数据
        // if (inputbuffer_.pickmessage(message) == false)
        //   break;
        // std::cout << "accept socket[" << fd() << "] recv(): " << message << std::endl;
        // 结束
        //lasttime_ = Timestamp::now(); // 更新connection的事件戳
        //LOG_INFO("更新connection的事件戳 ");
        // 处理到每组数据后执行发送回调
        if (onmessagecallback_)
        {
          //std::cout <<"回调onmessage-" <<std::endl;
          onmessagecallback_(shared_from_this(), &inputbuffer_);
        }

        else
        {
          std::cout << "-----------------Error: onmessagecallback_ is not set!" << std::endl;
        }
        break;
      }
      break;
    }
    else if (nread == 0) // 客户端连接已经断开
    {
      clientchannel_->remove(); // 从事件循化中删除channel
      //std::cout << " close back connection " << std::endl;
      closecallback();
      break;
    }
  }
}

// 发送缓冲区,并设置写事件，不管在任何线程中，都是调用此函数进行发送数据
void Connection::send(const char *data, size_t size)
{
  //std::cout <<__FUNCTION__ <<"---- current thread: " << syscall(SYS_gettid) << std::endl;
  if (disconnect_ == true)
  {
    //std::cout << "客户端连接断开，send()函数直接返回" << std::endl;
    return;
  }
  if (loop_->isinloopthread()) // 判断当前线程是否为IO线程
  {
    // 如果是IO线程，直接执行发送数据
    //std::cout << "IO线程，直接执行发送数据"<< std::endl;
    sendinloop(data, size);
  }
  else
  {
    // 如果当前线程不是I/O线程，把发送数据的操作交给IO线程处理,调用eventloop::queueinloop(),把sendinloop()交给事件循环中去执行
    //std::cout << "当前线程不是I/O线程" << std::endl;
    //sendinloop(data,size);
    // 不在自己的线程，需要移动
    // 方法一
    std::string send_data(data, size);
    //std::cout << "-------------"<<send_data << std::endl;
    loop_->queueinloop([send_data = std::move(send_data), this]() {
      sendinloop(send_data.data(), send_data.size());
    });
    // 方法二
    //loop_->queueinloop(std::bind(&Connection::sendinloop, this, std::move(data), size));
    
    // loop_->queueinloop([this, data = std::move(data),size](){
    //   sendinloop(data,size);
    // });
  }
}

// 执行发送数据，如果是IO线程，直接调用此函数，如果是工作线程，将此函数传给IO线程执行
void Connection::sendinloop(const char *data, size_t size)
{
  // 把需要发送的内容保存到Connection的发送缓冲区中
  // std::string ss1(data,size);
  //std::cout <<"Connection::sendinloop-------"  << std::endl;
  // 出错
  outbuffer_.appendwithsep(data, size);
  //std::cout << "----" << outbuffer_.buf_ <<std::endl;

  // 注册写事件
  this->clientchannel_->enablewriting();
}

void Connection::writecallback() // 写事件->I/O线程
{
  //LOG_INFO("发送----");
  //std::cout << outbuffer_.buf_<<std::endl;
  int writen = ::send(this->fd(), outbuffer_.data(), outbuffer_.size(), 0);
  //std::cout << outbuffer_.buf_<<std::endl;
  //LOG_INFO("发送完成，%d",writen);
  if (writen > 0)
    outbuffer_.erase(writen); // 写入数据后清除缓冲
  // 如果发送缓冲区没有数据，不再关闭写事件

  if (outbuffer_.size() == 0)
  {
    // std::cout << "关闭写事件" <<std::endl;
    clientchannel_->disablewriting();
    if(sendcomplate_)
      sendcomplate_(shared_from_this()); // 发送成功
    else{
      LOG_ERROR("%s %s %d send failed!",__FILE__,__FUNCTION__,__LINE__);
    }
  }
}
bool Connection::timeout(time_t now, int val) // 判断TCP连接是否超时，空闲太久
{
  //std::cout << "---qqqqq"<<this->fd()<<std::endl;
  return (now - lasttime_.toint()) > val;
}

void Connection::closecallback() // TCP连接关闭（断开）的回调函数，供channel回调
{
  //std::cout << " connection clsoe back " << std::endl;
  disconnect_ = true;

  this->loop_->erase(this->fd());
  closecallback_(shared_from_this());
}

void Connection::errorcallback() // TCP连接错误的回调函数，供channel回调
{
  disconnect_ = true;
  errorcallback_(shared_from_this());
}

void Connection::setsendcomplatecallback(const std::function<void(const spConnection &)> &fn)
{
  this->sendcomplate_ = std::move(fn);
}

void Connection::setclosecallback(const std::function<void(const spConnection &)> &fn)
{
  closecallback_ = std::move(fn);
}
void Connection::seterrorcallback(const std::function<void(const spConnection &)> &fn)
{
  errorcallback_ = std::move(fn);
}

void Connection::setonmessagecallback(const std::function<void(const spConnection &, Buffer*)> &fn)
{
  onmessagecallback_ = std::move(fn);
}

Connection::~Connection()
{
  //LOG_INFO("析构函数");
  //std::cout << " 析构函数 " << std::endl;
  //delete clientsock_;
  //delete clientchannel_;
}