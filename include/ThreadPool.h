/**
 * @file ThreadPool.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
 public:
  ThreadPool(size_t threads);
  ~ThreadPool();
  template <typename F, typename... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>;

 private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mtx_;
  std::condition_variable condition_;
  bool stop_;
};

inline ThreadPool::ThreadPool(size_t threads) : stop_(false) {
  // 设置线程任务
  for (size_t i = 0; i < threads; ++i) {
    // 每个线程需要从任务队列中获取任务 执行任务
    workers_.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->mtx_);
          // 等待唤醒 条件是停止或者任务队列中有任务
          this->condition_.wait(
              lock, [this] { return this->stop_ || !this->tasks_.empty(); });
          if (this->stop_ && this->tasks_.empty()) {
            return;
          }
          task = std::move(this->tasks_.front());
          this->tasks_.pop();
        }
        task();
      }
    });
  }
}

inline ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(mtx_);
    stop_ = true;
  }
  condition_.notify_all();
  for (auto& worker : workers_) {
    worker.join();
  }
}

template <typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;
  // 将需要执行的任务函数打包（bind），转换为参数列表为空的函数对象
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(mtx_);
    if (stop_) {
      throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    // 最妙的地方，利用 lambda函数 包装线程函数，使其符合 function<void()>
    // 的形式 并且返回值可以通过 future 获取
    tasks_.emplace([task]() { (*task)(); });
  }
  condition_.notify_all();
  return res;
}