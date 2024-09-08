#include "TcpServer.h"

// EventLoop* CheckLoopNotnull(EventLoop* loop){
//   if(loop == nullptr){
//     LOG_FATAL("%s:%s:%d mainloop is null!\n",__FILE__,__FUNCTION__,__LINE__);
//   }
//   return loop;
// }

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum,int buffer_sep) 
        : threadnum_(threadnum),buffer_sep_(buffer_sep), 
        mainloop_(new EventLoop(true,20,50)), 
        acceptor_(mainloop_.get(), ip, port), 
        threadpool_(threadnum_, "IO"),
        onmessagecb_(),
        sendcompletecb_()
{
  this->mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
  acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

  // 创建从事件循环
  for (int ii = 0; ii < threadnum_; ii++)
  {
    subloops_.emplace_back(new EventLoop(false,5,10));
    // 创建从事件循环，存入subloops_
    subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
    // 设置超时回调函数
    subloops_[ii]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1)); 
    // 配发任务
    threadpool_.addtask(std::bind(&EventLoop::run, subloops_[ii].get()));
    // 必须将只能指针转为普通指针
    //sleep(1);
  }
}

void TcpServer::start()
{
  mainloop_->run();
}

void TcpServer::stop() // 停止IO线程和事件循环
{
  // 停止主线程循环
  mainloop_->stop();
  std::cout << "主事件循环停止" << std::endl;
  // 停止从事件循环
  for(int ii=0;ii<threadnum_;ii++){
    subloops_[ii]->stop();
  }
  std::cout << "从事件循环停止" << std::endl;
  // 停止IO线程
  threadpool_.stop();
  std::cout << "I/O线程循环停止" << std::endl;

}

// 处理新客户端的请求
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
  // 创建connection
  spConnection conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock), buffer_sep_));
  conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
  conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
  if (newconnectioncb_) newconnectioncb_(conn);
  else
  {
    LOG_ERROR("%s %s error :%s ",__FILE__,__FUNCTION__, strerror(errno));
  }
  // 把conn存放到map容器中用于管理
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn; 
  }
  // 把conn存放到eventloop的map容器中,一个从事件循环可以有多个connection
  subloops_[conn->fd() % threadnum_]->newconnection(conn);
  // 回溯echoserver::handlenewconnction();
  conn->setonmessagecallback(onmessagecb_);
    //conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
  conn->setsendcomplatecallback(sendcompletecb_);
    //conn->setsendcomplatecallback(std::bind(&TcpServer::sendcomplate, this, std::placeholders::_1));
  conn->connectEstablished();      // 注册读事件
}

void TcpServer::closeconnection(const spConnection& conn) // 关闭客户端
{
  if (closeconnectioncb_) closeconnectioncb_(conn);
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_.erase(conn->fd());
  }
}
void TcpServer::errorconnection(const spConnection& conn) // 客户端的连接错误
{
  if (errorconnectioncb_)
    errorconnectioncb_(conn);
  //std::cout << "client[" << conn->fd() << "] error" << std::endl;
  //close(conn->fd()); // 关闭客户fd
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_.erase(conn->fd());
  }

  //delete conn;
}

// 删除conns_的connection对象，在eventloop::handletimer() 中回调此函数
void TcpServer::removeconn(int fd)
{
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_.erase(fd);
    // std::cout << "TcpServer::removeconn：  " << syscall(SYS_gettid) << std::endl;
  }
  if(removestatcallback_) removestatcallback_(fd);
}

// void TcpServer::onmessage(const spConnection& conn, std::string &message)
// {
//   std::cout <<"----------- onmessage : " <<syscall(SYS_gettid) <<std::endl;
//   // 处理收到的消息
//   if (onmessagecb_)
//     this->onmessagecb_(conn, message);
//   else
//   {
//     LOG_ERROR("%s %s error :%s ",__FILE__,__FUNCTION__, strerror(errno));
//   }
// }

// void TcpServer::sendcomplate(const spConnection& conn)
// {
//   //std::cout << "send success!" << std::endl;
//   if (sendcompletecb_)
//     sendcompletecb_(conn);
// }

void TcpServer::epolltimeout(EventLoop *loop)
{
  //std::cout << "epoll wait() timeout" << std::endl;
  if (timeoutcb_)
    timeoutcb_(loop);
}

void TcpServer::setnewconnectioncb(const std::function<void(const spConnection&)>& fn)
{
  newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(const std::function<void(const spConnection&)>& fn)
{
  closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(const std::function<void(const spConnection&)>& fn)
{
  errorconnectioncb_ = fn;
}

void TcpServer::setonmessagecb(const std::function<void(const spConnection&, Buffer *message)> &fn)
{
  onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(const std::function<void(const spConnection&)>& fn)
{
  sendcompletecb_ = fn;
}

void TcpServer::settimeoutcb(const std::function<void(EventLoop *)> &fn)
{
  timeoutcb_ = fn;
}

void TcpServer::setremovestatecb(const std::function<void(int)>& fn)
{
  removestatcallback_ = fn ;
}
TcpServer::~TcpServer()
{
  //delete acceptor_;
  //delete mainloop_;

  // 释放所有的connection对象
  // for (auto &aa:conns_)
  // {
  //   delete aa.second;
  // }
  //释放从事件循环
  // for(auto & aa:subloops_){
  //   delete aa;
  // }
  // delete threadpool_;   // 释放线程池。
}