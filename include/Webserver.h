#pragma
#include <sys/stat.h>
#include <functional>
#include <mysql/mysql.h>
#include <fstream>

#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "sql_connection_pool.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "HttpRequest.h"
class HttpRequest;
class HttpResponse;
using namespace std;

class Webserver : public noncopyable
{
private:
  using HttpCallback = std::function<void(const HttpRequest&,HttpResponse*)>;
  TcpServer tcpserver_;
  ThreadPool threadpool_; // 工作线程池

  ConnectionPool *connPool_; // 数据库相关
  string user_;              // 登陆数据库用户名
  string passwd_;            // 登陆数据库密码
  string databaseName_;      // 使用数据库名
  int sqlNum_;
  map<string, string> users;
  HttpCallback httpCallback_;
public:
  Webserver(const std::string &ip,
            const uint16_t port,
            const string &user,
            const string &passwd,
            const string &databaseName,
            int sqlNum,
            int subthreadnum = 3,
            int workthreadnum = 5,
            int buffer_sep = 1);
  ~Webserver();

  void start(); // 开启 服务
  void Stop();  // 停止服务

  // 处理新客户端的请求
  void HandleNewConnection(const spConnection &clientsock);
  void HandleCloseConnection(const spConnection &conn);               // 关闭客户端
  void HandleErrorConnection(const spConnection &conn);               // 客户端的连接错误
  void HandleMessage(const spConnection &conn, Buffer* message); // 处理收到的消息
  void HandlesendComplate(const spConnection &conn);                  // 数据发送完成过后回调
  void HandleEepollTimeout(EventLoop *loop);
  // 处理可客户端的请求报文，用于添加给线程池
  void OnMessage(const spConnection &conn, Buffer* message);
  void initmysql(ConnectionPool *connPool);
  void onRequest(const spConnection &, const HttpRequest &);
  void setHttpCallback(const HttpCallback& cb)
  {
    httpCallback_ = cb;
  }
  void onHttpProcess(const HttpRequest &req, HttpResponse *resp);



};