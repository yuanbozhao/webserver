// // 数据库配置信息
// g++ -o webserver ../src/Timestamp.cpp ../src/Logger.cpp ../src/sql_connection_pool.cpp ./websql.cpp  -lpthread -lmysqlclient

// 简单测试数据库

#include <map>
#include <iostream>

#include "sql_connection_pool.h"
using namespace std;
// 需要修改的数据库信息,登录名,密码,库名
string user = "webserver_zhao";
string password = "123456";
string databasename = "webserver";

map<string, string> users;

void initmysqlResult(ConnectionPool *connPool)
{
    // 先从连接池中取一个连接
    MYSQL *mysql = nullptr;
    ConnectionRAII mysqlcon(&mysql, connPool);

    // 在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,password FROM user"))
    {
      LOG_ERROR("SELECT error: %s",mysql_error(mysql));
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
}

int main(int argc, char const *argv[])
{
    // 初始化数据库连接池
    ConnectionPool *connPool = ConnectionPool::getInstance();
    connPool->init("127.0.0.2", user, password, databasename, 3306, 8);
    std::cout << "初始化完成" << std::endl;
    // 初始化数据库读取表
    initmysqlResult(connPool);
    for (auto iter = users.begin(); iter != users.end(); iter++)
    {
        cout << "username: " << iter->first << " password: " << iter->second << std::endl;
    }
    return 0;
}



// #include<iostream>
// #include<string>
// #include <mysql/mysql.h>
// #include "Logger.h"
// static std::string server = "127.0.0.2";
// static std::string user = "webserver_zhao";
// static std::string password = "123456";
// static std::string dbname = "webserver";

// class MySQL
// {
// public:
//   MySQL()
//   {
//     conn_ = mysql_init(nullptr);
//   }
//   ~MySQL()
//   {
//     if(conn_ !=nullptr)
//     {
//       mysql_close(conn_);
//     }
//   }
//   bool connect()
//   {
//     MYSQL* p = mysql_real_connect(conn_,server.c_str(),user.c_str(),
//                 password.c_str(),dbname.c_str(),3306,nullptr,0);
//     if(p != nullptr)
//     {
//       mysql_query(conn_, "set name gbk");
//     }
//     return p;
//   }
//   bool update(std::string sql)
//   {
//     if (mysql_query(conn_, sql.c_str()))
//     {
//       LOG_INFO("%s %s sql: %s 更新失败!",__FILE__ , __LINE__,sql.c_str());
//       return false;
//     } 
//     return true;
//   }
//   MYSQL_RES* query(std::string sql)
//   {
//     if (mysql_query(conn_, sql.c_str()))
//     {
//       LOG_INFO("%s %s sql: %s 查询失败!",__FILE__ , __LINE__,sql.c_str());
//       return nullptr;
//     } 
//     return mysql_use_result(conn_);
//   }

// private:
//   MYSQL* conn_;
// };

// int main()
// {

//   return 0;
// }