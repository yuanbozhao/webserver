#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>  // memory
#include <errno.h>
#include <unistd.h>
#include <netinet/tcp.h>  // tcp_nodelay

#include "Logger.h"
#include "inetAddress.h"
#include "noncopyable.h"

// 创建非阻塞监听的socket
int createnonblocking();

class Socket : noncopyable
{
private:
  const int fd_;      // fd
  std::string ip_;    // 存放ip地址
  uint16_t port_;     // 存放port

public:
  Socket(int fd);   
  ~Socket();         // 析构函数
  int fd() const;   //  返回fd成员
  std::string ip() const;     // 返回ip
  uint16_t port() const;      // 返回port

  void setreuseaddr(bool on);   // SO_REUSEADDR
  void setreuseport(bool on);   // SO_REUSEPORT
  void settcpnodelay(bool on);  // TCP_NODELAY
  void setkeepalive(bool on);   // SO_KEEPALIVE
  
  void setipport(const std::string& ip,const uint16_t port);
  void bind(const InetAddress& servaddr); // 服务端的socket调用
  int listen(int nn = SOMAXCONN);    
  int accept(InetAddress& clientaddr);     // 客户端的socket

};
