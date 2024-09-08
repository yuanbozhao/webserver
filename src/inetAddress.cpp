#include "inetAddress.h"

#include <string.h>


// 服务端功能 监听socket
InetAddress::InetAddress(const std::string &ip,uint16_t port){
  bzero(&addr_,sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = htonl(INADDR_ANY);;
  //addr_.sin_addr.s_addr = inet_addr(ip.data());
}

// 客户端功能
InetAddress::InetAddress(const sockaddr_in addr):addr_(addr){}

InetAddress::~InetAddress(){}

// 返回const char* ip
const char* InetAddress::ip() const{
  return inet_ntoa(addr_.sin_addr);
}

// 返回uint16_t port
uint16_t InetAddress::port() const{
  return ntohs(addr_.sin_port);
}

// 返回ipport
std::string InetAddress::ipport() const   
{
  char buf[64] = {0};
  inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  size_t end = strlen(buf);
  uint16_t port = ntohs(addr_.sin_port);
  snprintf(buf + end,sizeof buf,":%u",port);  // 更安全
  return buf;
}

// 返回 const inetaddr*
const sockaddr* InetAddress::addr() const{
  return (sockaddr*) &addr_;
}

void InetAddress::setaddr(sockaddr_in clientaddr){
  addr_= clientaddr;
}