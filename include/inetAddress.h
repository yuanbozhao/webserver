#pragma once

#include <arpa/inet.h>   //inet_addr

#include <string>
#include <iostream>

// socket 地址协议类

class InetAddress
{
private:
  sockaddr_in addr_;  // 表示地址协议的结构体
public:
  InetAddress() = default;
  explicit InetAddress(const std::string &ip,uint16_t port);  // 监听socket
  explicit InetAddress(const sockaddr_in addr);  // 客户端连接的socket

  ~InetAddress();

  const char* ip() const;   // 返回字符串的ip地址
  uint16_t port() const;    // 返回整数表示的端口号
  std::string ipport() const;   // 返回ipport
  const sockaddr* addr() const; // 转换为 sockaddr
  
  void setaddr(sockaddr_in clientaddr); // 设置addr_成员的值，赋值给该类
};
