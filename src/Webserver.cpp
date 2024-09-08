#include "Webserver.h"

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

Webserver::Webserver(const std::string &ip,
                     const uint16_t port,
                     const string &user,
                     const string &passwd,
                     const string &databaseName,
                     int sqlNum,
                     int subthreadnum,
                     int workthreadnum,
                     int buffer_sep)
    : tcpserver_(ip, port, subthreadnum, buffer_sep),
      user_(user), passwd_(passwd),
      databaseName_(databaseName), sqlNum_(sqlNum),
      threadpool_(workthreadnum, "WORKS")
{
  tcpserver_.setonmessagecb(std::bind(&Webserver::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
  tcpserver_.setcloseconnectioncb(std::bind(&Webserver::HandleCloseConnection, this, std::placeholders::_1));
  tcpserver_.seterrorconnectioncb(std::bind(&Webserver::HandleErrorConnection, this, std::placeholders::_1));
  tcpserver_.setsendcompletecb(std::bind(&Webserver::HandlesendComplate, this, std::placeholders::_1));
  tcpserver_.settimeoutcb(std::bind(&Webserver::HandleEepollTimeout, this, std::placeholders::_1));
  tcpserver_.setnewconnectioncb(std::bind(&Webserver::HandleNewConnection, this, std::placeholders::_1));
  this->setHttpCallback(std::bind(&Webserver::onHttpProcess, this, std::placeholders::_1, std::placeholders::_2));
}
Webserver::~Webserver() {}
void Webserver::start()
{
  connPool_ = ConnectionPool::getInstance();
  connPool_->init("127.0.0.1", user_, passwd_, databaseName_, 3306, sqlNum_);
  initmysql(connPool_);
  this->tcpserver_.start();
}
void Webserver::Stop() // 停止服务
{
  // 停止工作线程
  threadpool_.stop();
  std::cout << "Work线程循环停止" << std::endl;
  // 停止IO线程
  tcpserver_.stop();
}
// 处理新客户端的请求
void Webserver::HandleNewConnection(const spConnection &conn)
{

  //LOG_INFO("new connection(fd= %d,ip=%s,port=%d) ok.\n", conn->fd(), conn->ip().c_str(), conn->port());
  conn->setContext(HttpContext());
}
// 关闭客户端
void Webserver::HandleCloseConnection(const spConnection &conn)
{
  //LOG_INFO("connection closed(fd= %d,ip=%s,port=%d) ok.\n", conn->fd(), conn->ip().c_str(), conn->port());
  // {
  //   std::lock_guard<std::mutex> gd(mutex_);
  //   usermap_.erase(conn->fd());
  //   // std::cout << "close usermap_.erase(conn->fd())" << std::endl;
  // }
}
// 客户端的连接错误
void Webserver::HandleErrorConnection(const spConnection &conn)
{
  LOG_ERROR("connection closed(fd= %d,ip=%s,port=%d) error.\n", conn->fd(), conn->ip().c_str(), conn->port());
}

// 处理客户端请求报文，即收到的信息,在tcpserver类中回调
void Webserver::HandleMessage(const spConnection &conn, Buffer *message)
{
  //std::cout<<"HandleMessage(): " << syscall(SYS_gettid)<<this->usermap_.size()<< "usermap_.begin():"<<  usermap_.begin()->first<<std::endl;
  // 处理收到的消息
  if (threadpool_.size() == 0) // 工作线程为0
  {
    this->OnMessage(conn, message);
  }
  else
  {
    // 把任务添加到线程池的任务队列中
    threadpool_.addtask(std::bind(&Webserver::OnMessage, this, conn, message));
    //std::cout << message << std::endl;
  }
}

// 业务层面，负责计算
void Webserver::OnMessage(const spConnection &conn, Buffer *message)
{
  // 处理收到的消息
  //std::cout << message->buf_ << std::endl;
  //LOG_INFO("---开始发送消息");
  Timestamp lasttime = Timestamp::now(); // 更新connection的事件戳
  HttpContext *context = std::any_cast<HttpContext>(conn->getMutableContext());
  std::string receivemessage;
  if (!context->parseRequest(message, lasttime))
  {
    //std::cout << "HTTP/1.1 400 Bad Request" << std::endl;
    receivemessage = "HTTP/1.1 400 Bad Request\r\n\r\n";
    conn->send(receivemessage.data(), receivemessage.size());
    conn->closecallback(); // 关闭
  }
  if (context->gotAll()) // HTTP 请求完成
  {
    //std::cout << "请求完成" <<std::endl;
    onRequest(conn, context->request());
    context->reset();
  }
  else
  {
    LOG_ERROR("%s %s context->gotAll()：error： %s\n",__FILE__,__FUNCTION__,strerror(errno));
  }
  
}

// 数据发送完成过后回调
void Webserver::HandlesendComplate(const spConnection &conn)
{
  //LOG_INFO("Message send complete.");
}

void Webserver::HandleEepollTimeout(EventLoop *loop)
{
  //std::cout << "Webserver timeout." << std::endl;
}

void Webserver::onRequest(const spConnection &conn, const HttpRequest &req)
{
  const string &connection = req.getHeader("Connection");
  bool close = connection == "close" ||
               (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(buf.data(), buf.size());
  if(close) conn->closecallback();
}

void Webserver::onHttpProcess(const HttpRequest &req, HttpResponse *resp)
{
  //LOG_INFO("Headers ： %s %s ", req.methodString(), req.path().c_str());
  std::string file;
  int state = -1;
  if (!req.body().empty())
  {
    //std::cout << " body 不为空" <<std::endl;
    const char *bodyStr = req.body().c_str();
    // 将用户名和密码提取出来
    // user=123&passwd=123
    char name[100], password[100];
    int i;
    for (i = 5; bodyStr[i] != '&'; ++i)
      name[i - 5] = bodyStr[i];
    name[i - 5] = '\0';

    int j = 0;
    for (i = i + 10; bodyStr[i] != '\0'; ++i, ++j)
      password[j] = bodyStr[i];
    password[j] = '\0';
    if (req.path() == "/3CGISQL.cgi")
    {
      // 表示注册
      if (users.count(name))
      {
        file.append("../resources/registerError.html");
      }
      else
      {
        // 如果是注册，先检测数据库中是否有重名的
        // 没有重名的，进行增加数据
        char *sql_insert = (char *)malloc(sizeof(char) * 200);
        strcpy(sql_insert, "INSERT INTO user(username, password) VALUES(");
        strcat(sql_insert, "'");
        strcat(sql_insert, name);
        strcat(sql_insert, "', '");
        strcat(sql_insert, password);
        strcat(sql_insert, "')");

        // 先从连接池中取一个连接
        MYSQL *mysql = NULL;
        ConnectionRAII mysqlcon(&mysql, connPool_);
        // 此处感觉需要锁一下
        int res = mysql_query(mysql, sql_insert);
        if (!res)
        {
          users[name] = password;
          file.append("resources/login.html");
        }
        else
        {
          file.append("resources/registerError.html");
        }
      }
    }
    else if (req.path() == "/2CGISQL.cgi")
    {
      // 表示登录
      if (users.count(name) && users[name] == password)
      {
        file.append("../resources/welcome.html");
      }
      else
      {
        file.append("../resources/logError.html");
      }
    }
  }
  else if (req.path() == "/")
  {
    //std::cout << "../resources/judge.html" << std::endl;
    file.append("../resources/judge.html");
  }
  else if(req.path() == " /favicon.ico")
  {
    state = 1;
    file.append("../resources/favicon.ico");
  }
  else if (req.path() == "/0")
  {
    file.append("../resources/register.html");
  }
  else if (req.path() == "/1")
  {
    file.append("../resources/log.html");
  }
  else if (req.path() == "/5")
  {
     state = 5;
    file.append("../resources/picture.html");
  }
  else if (req.path() == "/6")
  {
     state = 6;
    file.append("../resources/video.html");
  }
  else if (req.path() == "/7")
  {
     state = 7;
    file.append("../resources/fans.html");
  }
  else if (req.path() == "/404")
  {
    file.append("../resources/404.html");
  }
  else
  {
    // strcpy(file, "resources");
    file.append("resources");
    // int len = strlen(file);
    const char *url_real = req.path().c_str();
    file.append(url_real);
  }

  // 读取文件状态
  struct stat fileStat;
  if (stat(file.c_str(), &fileStat) < 0)
  {
    //std::cout << file << " no such file" << std::endl;
    file.clear();
    file.append("resources/404.html");
    stat(file.c_str(), &fileStat);
  }
  //std::cout << file << std::endl;
  if (!(fileStat.st_mode & S_IROTH))
    return;

  if (S_ISDIR(fileStat.st_mode))
    return;
  resp->setStatusCode(HttpResponse::k200Ok);
  resp->setStatusMessage("OK");
  resp->addHeader("Server", "tinyMuduo");
  //resp->setContentType("text/html");
  resp->setFile(file);

  switch(state){
    case 1:
      resp->setContentType("image/x-icon");
      break;
    case 5:
      resp->setContentType("image/jpg");
      break;
    case 6:
      resp->setContentType("video/mp4");
      break;
    case 7:
      resp->setContentType("image/jpg");
      break;
    default:
      resp->setContentType("text/html");
  }
  //std::cout << "解析完成呢个" <<std::endl;
}

// 初始化数据库
void Webserver::initmysql(ConnectionPool *connPool)
{
  // 先从连接池中取一个连接
  MYSQL *mysql = NULL;
  ConnectionRAII mysqlcon(&mysql, connPool);
  // 在user表中检索username，passwd数据，浏览器端输入
  if (mysql_query(mysql, "SELECT username,password FROM user"))
  {
    LOG_ERROR("SELECT error: %s", mysql_error(mysql));
  }

  // 从表中检索完整的结果集
  MYSQL_RES *result = mysql_store_result(mysql);

  // 返回结果集中的列数
  int num_fields = mysql_num_fields(result);

  // 返回所有字段结构的数组
  MYSQL_FIELD *fields = mysql_fetch_fields(result);

  // 从结果集中获取下一行，将对应的用户名和密码，存入map中
  while (MYSQL_ROW row = mysql_fetch_row(result))
  {
    string temp1(row[0]);
    string temp2(row[1]);
    users[temp1] = temp2;
  }
  LOG_INFO("Init mysql success");
}