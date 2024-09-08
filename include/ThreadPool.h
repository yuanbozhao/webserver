#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unistd.h>
#include <future>
#include <atomic>
#include <queue>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/syscall.h>
class ThreadPool{
private:
  std::vector<std::thread> threads_;      //线程池中的线程
  std::queue<std::function<void()>> taskqueue_;  // 任务队列
  std::mutex mutex_;                             // 任务队列的同步锁
  std::condition_variable condition_;            // 任务队列同步的条件变量
  std::atomic_bool stop_;                        // 析构函数，把stop的值设置为true,全部的线程将推出
  std::string threadtype_;                  // 线程种类：“IO"，”WORKS“
public:
  ThreadPool(size_t threadnum, const std::string& threadtype);
  // 添加线程队列
  void addtask(std::function<void()>task);
  // 获取线程池的大小
  size_t size();
  
  void stop();  // 停止线程
  ~ThreadPool();

};
