/**
 * @file Timer.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-08
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

class Timer {
 public:
  Timer() : expired_(true), try_to_expire_(false) {}
  ~Timer() {}
  void start(int interval, std::function<void()> task);
  void stop();

 private:
  std::condition_variable cv_;
  std::mutex mtx_;
  std::atomic<bool> expired_;
  std::atomic<bool> try_to_expire_;
};

inline void Timer::start(int interval, std::function<void()> task) {
  if (!expired_) {
    return;
  }
  expired_ = false;
  std::thread([this, interval, task]() {
    while (!this->try_to_expire_) {
      std::this_thread::sleep_for(std::chrono::microseconds(interval));
      task();
    }
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->expired_ = true;
    this->cv_.notify_one();
  }).detach();
}

inline void Timer::stop() {
  if (expired_ == true) {
    return;
  }
  if (try_to_expire_ == true) {
    return;
  }
  try_to_expire_ = true;
  std::unique_lock<std::mutex> lock(mtx_);
  cv_.wait(lock, [this] { return this->expired_ == true; });
  if (expired_ == true) {
    try_to_expire_ = false;
  }
}