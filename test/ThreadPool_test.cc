/**
 * @file ThreadPool_test.cc
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "../include/ThreadPool.h"

#include <iostream>

int main(int argc, char* argv[]) {
  ThreadPool myPool(4);
  std::vector<std::future<int>> results;

  for (int i = 0; i < 8; ++i) {
    results.emplace_back(myPool.enqueue([i]() {
      std::cout << "hello " << i << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "world " << i << std::endl; 
      return i;
    }));
  }
 
  for (auto&& result : results) std::cout << result.get() << ' ';
  std::cout << std::endl;
}