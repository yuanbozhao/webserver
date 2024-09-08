#include "Accpetor.h"
#include "Connection.h"

Acceptor::Acceptor(EventLoop* loop,const std::string &ip, const uint16_t port)
      :loop_(loop),
      servsock_(createnonblocking()),
      acceptchannel_(loop_,servsock_.fd()){
  //servsock_ = new Socket((createnonblocking()));
  // bind socket
  //std::cout << "acceptchannel_: "<<acceptchannel_.fd() <<std::endl;
  servsock_.setkeepalive(true);
  servsock_.setreuseaddr(true);
  servsock_.setreuseport(true);
  servsock_.settcpnodelay(true);

  // int bufsize;
  // socklen_t optlen = sizeof(bufsize);
  // getsockopt(servsock_->fd(), SOL_SOCKET,SO_SNDBUF,&bufsize, &optlen); 
  // // 获取发送缓冲区的大小
  // std::cout << "send buffer size: " << bufsize<<std::endl;
  // getsockopt(servsock_->fd(), SOL_SOCKET,SO_RCVBUF,&bufsize, &optlen); 
  // // 获取接收缓冲区的大小
  // std::cout << "recv buffer size: " << bufsize<<std::endl;
  InetAddress servaddr(ip,port);  
  servsock_.bind(servaddr);
  // listen
  servsock_.listen();

  //std::cout << "socket[" << acceptchannel_.fd() << "]" << "listen..." << std::endl;
  
  // 添加事件
  //acceptchannel_ = new Channel(loop_,servsock_.fd());
  // 使用回调函数处理连接
  acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection,this));
  acceptchannel_.useet(); 
  acceptchannel_.enablereading();  // 注册读事件  // channel =>poller

}
void Acceptor::newconnection() // 处理新客户端的请求
{
  while(true)
  {
    // accpet
    InetAddress clientaddr;
    int accept_fd =servsock_.accept(clientaddr);
    if(accept_fd < 0)
    {
      if(errno == EAGAIN)
        break;
      else{
        LOG_FATAL("%s %s errno = %s",__FILE__,__FUNCTION__,strerror(errno));
      }
    }
    std::unique_ptr<Socket> clientsock(new Socket(accept_fd));
    if(clientsock->fd() >0){
      if(newconnectioncb_){
        //std::cout << " newconnection--- 调用" << std::endl;
        clientsock->setipport(clientaddr.ip(),clientaddr.port());
        // 注意：clientsock 只能new 出来，不能在栈上，否则析构函数会关闭fd
        // 回调新的客户端 TcpServer::newconnection
        newconnectioncb_(std::move(clientsock));     
      }
      else{
        ::close(clientsock->fd());
      }
    }
    else{
      LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__,__FUNCTION__,__LINE__,errno);
    }
  }
}

void Acceptor::setnewconnectioncb(const std::function<void( std::unique_ptr<Socket>)>&fn)
{
  newconnectioncb_ = std::move(fn);
}

Acceptor::~Acceptor(){
  //delete servsock_;
 // delete acceptchannel_;
}