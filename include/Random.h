/**
 * @file Random.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <cstdint>

class Random {
 public:
  explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
    // Avoid bad seeds
    if (seed_ == 0 || seed_ == 0x7fffffff) {
      seed_ = 1;
    }
  }
  // seed = (seed * A + C)%M (C=0)
  uint32_t Next() {
    static const uint32_t M = 2147483647L;  // 2^31-1
    static const uint32_t A = 16807;        // bits 14 8 7 5 2 1 0
    uint64_t product = seed_ * A;
    // product % M=(product>>31)+(product & M)
    seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
    if (seed_ > M) {
      seed_ -= M;
    }
    return seed_;
  }
  // value in the range [0,n-1]
  uint32_t Uniform(int n) { return Next() % n; }
  // judge Uniform is zero
  bool OneIn(int n) { return (Next() % n) == 0; }
  // value in the range [0,2^max_log-1]
  uint32_t Skewed(int max_log) { return Uniform(1 << Uniform(max_log + 1)); }

 private:
  uint32_t seed_;
};