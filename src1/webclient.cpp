// webserver
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "Logger.h"

// sep =>1  发送 报头+报文
ssize_t tcpsend(int fd, void *data, size_t size)
{
  char tempbuf[1024];
  memset(tempbuf, 0, sizeof(tempbuf));
  memcpy(tempbuf, &size, 4);
  memcpy(tempbuf + 4, data, size);
  return send(fd, tempbuf, size + 4, 0);
}

// sep =>1  接收 报头+报文
ssize_t tcprecv(int fd, void *data)
{
  int len;
  if (recv(fd, &len, 4, 0) < 0)
  {
    LOG_ERROR("%s %s errno:%s", __FILE__, __FUNCTION__, strerror(errno));
  }
  return recv(fd, data, len, 0);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << "use ip port" << std::endl;
    std::cout << "example: ./client 127.0.0.1 8888" << std::endl;
    return -1;
  }

  int sockfd;
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd <= 0)
  {
    LOG_FATAL("%s %s %s errno:%s", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
  }
  //std::cout << "creat socket[" << sockfd << "] success" << std::endl;
  // int bufsize;
  // socklen_t optlen = sizeof(bufsize);
  // getsockopt(sockfd, SOL_SOCKET,SO_SNDBUF,&bufsize, &optlen);
  // // 获取发送缓冲区的大小
  // std::cout << "send buffer size: " << bufsize<<std::endl;
  // getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF,&bufsize, &optlen);
  // // 获取接收缓冲区的大小
  // std::cout << "recv buffer size: " << bufsize<<std::endl;

  sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    LOG_FATAL("%s %s %s errno:%s", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
  }

  //std::cout << "connect ok" << std::endl;
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, 1024, "<bizcode>00101</bizcode><username>zhao</username><password>123</password>");
  if (tcpsend(sockfd, buffer, strlen(buffer)) <= 0)
  {
    std::cout << "send failed" << std::endl;
    return -1;
  }
  LOG_INFO("send : %s \n", buffer);
  memset(buffer, 0, sizeof(buffer));
  if (tcprecv(sockfd, buffer) <= 0)
  {
    std::cout << "recv failed" << std::endl;
    return -1;
  }
  else LOG_INFO("recv : %s \n", buffer);
  
  // 心跳报文
  while(true)
  {
    memset(buffer,0,sizeof(buffer));
    snprintf(buffer,1024,"<bizcode>00001</bizcode>");
    if(tcpsend(sockfd,buffer,strlen(buffer)) <= 0)
    {
      std::cout << "send 心跳 failed" <<std::endl;
    }
    memset(buffer,0,sizeof(buffer));
    if(tcprecv(sockfd,buffer) <=0)
    {
      std::cout << "recv 心跳 failed" << std::endl;
    }
    sleep(5);  // 5 秒发送一次
  }


  // 注销业务
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, 1024, "<bizcode>00901</bizcode>");
  if (tcpsend(sockfd, buffer, strlen(buffer)) <= 0)
  {
    std::cout << "send failed" << std::endl;
    return -1;
  }
  else
    LOG_INFO("send: %s \n", buffer);

  memset(buffer, 0, sizeof(buffer));
  if (tcprecv(sockfd, buffer) <= 0)
  {
    std::cout << "recv failed" << std::endl;
    return -1;
  }
  else LOG_INFO("recv: %s \n", buffer);
  return 0;
}