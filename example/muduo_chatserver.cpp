#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <functional>

using namespace std::placeholders;

class Chatserver
{
private:
  muduo::net::TcpServer server_;
  muduo::net::EventLoop *loop_;
  void onConnection(const muduo::net::TcpConnectionPtr &conn);
  void onMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buf);

public:
  Chatserver(muduo::net::EventLoop *loop,
             const muduo::net::InetAddress &listenaddr,
             const std::string &nameAge);
  void start();           
  ~Chatserver();
  
};

Chatserver::Chatserver(muduo::net::EventLoop *loop,
           const muduo::net::InetAddress &listenaddr,
           const std::string &nameAge)
           :server_(loop,listenaddr,nameAge),
           loop_(loop)
{
  // 用户注册链接的 创建和断开回调
    server_.setConnectionCallback(std::bind(&Chatserver::onConnection,this,_1));
  // 读写事件回调
    server_.setMessageCallback(std::bind(&Chatserver::onMessage,this,_1,_2));
}

Chatserver::~Chatserver()
{
}

void Chatserver::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void Chatserver::onMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buf)
{
   
}

void Chatserver::start()
{
  server_.start();
}  

int main()
{
  LOG_INFO << "pid = " << getpid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(8888);
  Chatserver server(&loop, listenAddr,"web");
  server.start();
  loop.loop();
}