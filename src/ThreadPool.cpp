#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadnum, const std::string &threadtype) : stop_(false), threadtype_(threadtype)
{
  // 启动线程
  for (size_t ii = 0; ii < threadnum; ii++)
  {
    // 用lambda创建线程
    threads_.emplace_back([this] 
    {
      std::cout << "create " << threadtype_.c_str()
                << " thread[" << syscall(SYS_gettid)
                << "]. "
                << std::endl; // 显示线程id
      while (stop_ == false)
      {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->mutex_);
          // 等待生产者的条件变量
          this->condition_.wait(lock, [this] {
            return (this->stop_ || !this->taskqueue_.empty());
          });
          // 如果队列中有任务，执行
          // stop_ 为 true 或者 taskqueue_不为空继续执行
          if (this->taskqueue_.empty() && (this->stop_ == true))
            return;
          // 出队任务执行
          task = std::move(this->taskqueue_.front()); // 移动语义，避免拷贝
          this->taskqueue_.pop();
          //std::cout << "在锁执行任务" << std::endl;
          //task();
        }
        task(); // 执行任务
      }
    });
  }
}

ThreadPool::~ThreadPool()
{
  stop();
}


void ThreadPool::addtask(std::function<void()> task)
{
  // 简单模式
  {
    std::lock_guard<std::mutex> lock(mutex_);
    taskqueue_.push(task);
  }

  condition_.notify_one(); // 唤醒一个线程
  // 简单模式结束
}

size_t ThreadPool::size()
{
  return this->threads_.size();
}

void ThreadPool::stop() // 停止线程
{
  if (stop_)
    return;
  stop_ = true;            // 原子操作，不会被线程调度机制打断
  condition_.notify_all(); // 唤醒所有在等待队列中的线程
  // 等待全部线程执行完成后推出
  for (std::thread &th : threads_)
  {
    if (th.joinable())
    {
      th.join();
    }
  }
}
