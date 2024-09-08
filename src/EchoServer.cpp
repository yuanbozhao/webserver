// #include "EchoServer.h"

// EchoServer::EchoServer(const std::string &ip, const uint16_t port, int subthreadnum, int workthrreadnum, int buffer_sep)
//     : tcpserver_(ip, port, subthreadnum, buffer_sep), threadpool_(workthrreadnum, "WORKS")
// {
//   tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
//   tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleCloseConnection, this, std::placeholders::_1));
//   tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleErrorConnection, this, std::placeholders::_1));
//   tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
//   tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandlesendComplate, this, std::placeholders::_1));
//   tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleEepollTimeout, this, std::placeholders::_1));
// }

// EchoServer::~EchoServer()
// {
// }

// void EchoServer::start()
// {
//   this->tcpserver_.start();
// }

// void EchoServer::Stop() // 停止服务
// {
//   // 停止工作线程
//   threadpool_.stop();
//   std::cout << "Work线程循环停止" << std::endl;
//   // 停止IO线程
//   tcpserver_.stop();
// }

// // 处理新客户端的请求
// void EchoServer::HandleNewConnection(const spConnection &conn)
// {
//   // mainloop_调用
//   //std::cout << "thread id: "<<syscall(SYS_gettid) << std::endl;

//   LOG_INFO("new connection(fd= %d,ip=%s,port=%d) ok.\n", conn->fd(), conn->ip().c_str(), conn->port());

//   //printf("%s new connection(fd= %d,ip=%s,port=%d) ok.\n", Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
//   //std::cout << "EchoServer::HandleNewConnection() thread is: 【" << syscall(SYS_gettid) << "】" << std::endl;
//   //std::cout << "new connection come in." << std::endl;
// }

// // 关闭客户端
// void EchoServer::HandleCloseConnection(const spConnection &conn)
// {
//   LOG_INFO("connection closed(fd= %d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
//   // std::cout << " conn close" << std::endl;
// }

// // 客户端的连接错误
// void EchoServer::HandleErrorConnection(const spConnection &conn)
// {
//   LOG_ERROR("connection (fd= %d,ip=%s,port=%d) error.\n", conn->fd(), conn->ip().c_str(), conn->port());
//   //std::cout << "EchoServer conn error." << std::endl;
// }


// // 处理客户端请求报文，即收到的信息,在tcpserver类中回调
// void EchoServer::HandleMessage(const spConnection &conn, std::string &message)
// {
//   //std::cout << "EchoServer::HandleMessage() thread is: " << syscall(SYS_gettid) << std::endl;
//   // 处理收到的消息
//   if (threadpool_.size() == 0) // 工作线程为0
//   {
//     this->OnMessage(conn, message);
//   }
//   else
//   {
//     // 把任务添加到线程池的任务队列中
//     threadpool_.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
//     //std::cout << message << std::endl;
//   }
// }

// // 业务层面，负责计算
// void EchoServer::OnMessage(const spConnection &conn, std::string &message)
// {
//   // 处理收到的消息

//   //LOG_INFO("%s message(fd=%d):%s\n",Timestamp::now().tostring().c_str(),conn->fd(),message.c_str());

//   message = "reply:" + message;
//   conn->send(message.data(), message.size());
//   // 处理完后工作线程是否释放了？？？ 
// }

// // 数据发送完成过后回调 => 肯定是I/O线程 
// void EchoServer::HandlesendComplate(const spConnection &conn)
// {
//   // std::cout << " thead id :" << syscall(SYS_gettid) << std::endl;
//   // std::cout << "Message send complete." << std::endl;
// }

// void EchoServer::HandleEepollTimeout(EventLoop *loop)
// {
//   //std::cout << "Echoserver timeout." << std::endl;
// }