// #pragma
// #include "TcpServer.h"
// #include "EventLoop.h"
// #include "Connection.h"
// #include "ThreadPool.h"
// class EchoServer:public noncopyable
// {
// private:
//   TcpServer tcpserver_;
//   ThreadPool threadpool_;  // 工作线程池

// public:
//   EchoServer(const std::string &ip, const uint16_t port,int subthreadnum=3,int workthreadnum =5,int buffer_sep=1);
//   ~EchoServer();

//   void start();   // 开启 服务
//   void Stop();     // 停止服务
  
//   // 处理新客户端的请求
//   void HandleNewConnection(const spConnection & clientsock);           
//   void HandleCloseConnection(const spConnection& conn);           // 关闭客户端
//   void HandleErrorConnection(const spConnection& conn);           // 客户端的连接错误
//   void HandleMessage(const spConnection& conn,std::string& message); // 处理收到的消息
//   void HandlesendComplate(const spConnection& conn);             // 数据发送完成过后回调
//   void HandleEepollTimeout(EventLoop* loop);

//   // 处理可客户端的请求报文，用于添加给线程池           
//   void OnMessage(const spConnection& conn,std::string& message);  
// };