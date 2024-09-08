
#include <cstddef>
#include <signal.h>

#include "Webserver.h"
// #include "EchoServer.h"

// EchoServer * echoserver;
Webserver * webserver;
void Stop(int sig)      // 信号2和信号15的处理函数，功能是停止服务程序
{
  printf("sig=%d\n", sig);
  webserver->Stop();
  delete webserver;
  exit(0);
}

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    std::cout << "use ip port I/O(thread) Work(thread)" << std::endl;
    std::cout << "example: ./webserver 127.0.0.1 8888" << std::endl;
    return -1;
  }

  signal(SIGTERM, Stop);         //2
  signal(SIGINT, Stop);          //15

  // 需要修改的数据库信息,登录名,密码,库名
  string user = "webserver_zhao";
  string passwd = "123456";
  string databasename = "webserver";
  int sqlNum = 8;     // sql 库

  // IO线程，word线程，sep_字段
  int IO_threadnum = 30;
  int WORK_threadnum = 0;
  int sep = 0;  // sep_字段
  // 0：默认字段
  // 1: 加入包头，用于防止TCP粘包问题
  webserver = new Webserver(argv[1],atoi(argv[2]),user,passwd,databasename,sqlNum,IO_threadnum,WORK_threadnum,sep);
  webserver->start();
 
  return 0;
}