#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <boost/bind/bind.hpp>
#include <muduo/net/EventLoop.h>
#include <functional>

using namespace std::placeholders;
class EchoServer
{
public:
  EchoServer(muduo::net::EventLoop *loop,
             const muduo::net::InetAddress &listenaddr);
  void start();

private:
  void onConnection(const muduo::net::TcpConnectionPtr &conn);
  void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time);
  muduo::net::TcpServer server_;
};

EchoServer::EchoServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenaddr)
          :server_(loop, listenaddr,"Echoserver")
{
  server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,_1));
  server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,_1,_2,_3));
}

void EchoServer::start()
{
  server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time)
{
  // 接收到所有的消息，然后回显
  muduo::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  conn->send(msg);
}

int main()
{
  LOG_INFO << "pid = " << getpid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(8888);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();    // epoll_wait以阻塞方式等待新用户连接
}