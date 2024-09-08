#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
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
    std::cout << "create socket failed" << std::endl;
    return -1;
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
    std::cout << "connect failed" << std::endl;
    return -1;
  }

  //std::cout << "connect ok" << std::endl;
  char buffer[1024];
  printf("开始事件：%d\n", time(0));
  for (int ii = 0; ii < 1000000; ii++)
  {
    //从命令行输入内容
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "这是第%d个超级女生", ii);

    char tempbuf[1024];
    memset(tempbuf, 0, sizeof(tempbuf));
    int len = strlen(buffer);
    memcpy(tempbuf, &len, 4);
    memcpy(tempbuf + 4, buffer, len);

    // std::cout << "please input:";
    // std::cin >> buffer;
    if (send(sockfd, tempbuf, len + 4, 0) <= 0)
    { // 把请求报文发送给服务端
      std::cout << "send() failed" << std::endl;
      close(sockfd);
      return -1;
    }
  // }

  // for (int ii = 0; ii < 100; ii++)
  // {
     len = 0;
    if (recv(sockfd, &len, sizeof(len), 0) < 0)
    {
      std::cout << "recv() buffer head falied" << std::endl;
    }

    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, len, 0) <= 0)
    {
      std::cout << "recv() falied" << std::endl;
    }

    //std::cout << " recv: " << buffer << std::endl;
  }
  printf("结束时间：%d\n", time(0));

  // for (int ii = 0; ii < 100; ii++)
  // {
  //   memset(buffer, 0, sizeof(buffer));
  //   sprintf(buffer, "这是第%d个超级女生", ii);
  //   send(sockfd, buffer, strlen(buffer), 0);
  //   // memset(buffer,0, sizeof(buffer));
  //   // recv(sockfd,buffer,1024,0);

  //   //std::cout << "recv:" << buffer << std::endl;
  //   //sleep(1);
  // }
  // for (int ii = 0; ii < 100; ii++)
  // {
  //   // memset(buffer,0,sizeof(buffer));
  //   // sprintf(buffer,"这是第%d个超级女生",ii);
  //   // send(sockfd,buffer,strlen(buffer),0);
  //   memset(buffer, 0, sizeof(buffer));
  //   recv(sockfd, buffer, 1024, 0);

  //   std::cout << "recv:" << buffer << std::endl;
  //   //sleep(1);
  // }
  //printf("结束时间：%d\n", time(0));
  //sleep(100);   // 100s

  return 0;
}