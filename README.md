# webserver

该项目是为了实现高并发的网络模型， 使用主从REACTOR模式对IO多路复用进行了封装。通过采用epoll+边沿触发+非阻塞的模型来实现对客户端事件的监听以及响应，为了实现IO处理与业务逻辑的分离，在应用层采用了工作线程池来单独处理业务逻辑，当工作线程运行完成，采用了eventfd作为事件通知描述符去通知IO线程。在高并发的场景中，利用timerfd实现了定时器功能区清除空闲连接。对于日志的显示采用了基于阻塞队列的异步日志系统。

## 编译方式

### 方式一

```
sh ./build.sh
```

### 方式二

```
 rm -rf *
 cmake ..
 make
```

## 服务器环境

```
 ubuntu 版本为: 18.04.1-Ubuntu 
 mysql  版本为：14.14-mysql
```

## 数据库配置

```
// 创建mysql用户
create user 'webserver_zhao' @ '%' identified by '123456';
// 创建数据库
create database webserver;
// 创建用户表
use webserver;
create table user(
	username char(50) null,
	password char(50) null
	)engine=InnoDB;
// 添加用户
insert into user(username,password) values('yourname','yourpassword');
```

## main.cpp 配置

```
// 需要修改的数据库信息,登录名,密码,库名
string user = "webserver_zhao";
string passwd = "123456";
string databasename = "webserver";
int sqlNum = 8;     // sql 库
// IO线程，word线程，sep_字段
int IO_threadnum = 30;
int WORK_threadnum = 0;
```

## 编译命令

在bin目录下： webserver为服务端，webclient 为客户端, clientup.sh 为高并发脚本测试，在webserver项目中不需要，client可以为浏览器

### 启动webserver

```
./webserver 127.0.0.1 8888
// 当前echoserver 支持任意Ip地址, 可以部署到云上
```

### 打开浏览器

```
127.0.0.1:8888
```

## 压力测试

```
./webbench  -c 1000 -t 60   http://127.0.0.1:8888/
```

测试结果

Webbench对服务器进行压力测试，经压力测试可以实现上万的并发连接.
> * 并发连接总数：1000
> * 访问服务器时间：60s
> * 每秒钟响应请求数：2391834 pages/min
> * 每秒钟传输数据量：331248 bytes/sec
> * 所有访问均成功

## 致谢

该项目参考了muduo网络库项目与TinyWebServer项目~
