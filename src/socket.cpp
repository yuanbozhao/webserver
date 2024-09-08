#include "socket.h"
#include<fcntl.h>


// void setnonblock(int fd){
//   fcntl(fd,F_SETFL,fcntl(fd, F_GETFL| O_NONBLOCK));
// }

int createnonblocking()
{
  int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (listenfd < 0)
  {
    std::cout << __FILE__ << ":"
              << __LINE__
              << "listen socket create error"
              << errno
              << std::endl;
    exit(-1);
  }
  return listenfd;
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket()
{
  //LOG_INFO("fd: %d 析构",fd_);
  ::close(fd_);
} 

int Socket::fd() const
{
  return fd_;
}
std::string Socket::ip() const{
  return ip_;
}

uint16_t Socket::port() const // 返回port
{
  return port_;
}


void Socket::setreuseaddr(bool on) // SO_REUSEADDR
{
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setreuseport(bool on) // SO_REUSEPORT
{
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::settcpnodelay(bool on) // TCP_NODELAY
{
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}
void Socket::setkeepalive(bool on) // SO_KEEPALIVE
{
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setipport(const std::string& ip,const uint16_t port){
  this->ip_ = ip;
  this->port_ = port;
}

void Socket::bind(const InetAddress &servaddr)
{
  if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0)
  {
    close(fd_);
    LOG_FATAL("bind() faliled: %s",strerror(errno));
  }
  setipport(servaddr.ip(),servaddr.port());
}

int Socket::listen(int nn)
{
  if (::listen(fd_, nn) != 0)   // SOMAXCONN
  {
    close(fd_);
    LOG_FATAL("listen() failed");
  }
}

int Socket::accept(InetAddress &peeraddr)
{
  sockaddr_in addr;
  bzero(&addr,sizeof addr);
  socklen_t len = sizeof(addr);
  int clientfd = accept4(fd_, (struct sockaddr *)&addr, &len, SOCK_NONBLOCK);
  if(clientfd >=0) peeraddr.setaddr(addr);

  //ip_ = std::string(clientaddr.ip());  // 将客户端的ip和port绑定到serverfd不对
  //port_ = clientaddr.port();

  // 只有服务端会调用该函数，因此这个Ip和port是服务端的
  return clientfd;
}