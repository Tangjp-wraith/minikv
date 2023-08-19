#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

// 内存管理类
class Arena {
 public:
  Arena() : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}
  ~Arena() {
    for (size_t i = 0; i < blocks_.size(); ++i) {
      delete[] blocks_[i];
    }
  }
  // 删除拷贝构造
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  // Get 用于测试
  size_t GetRemainBytes() { return alloc_bytes_remaining_; }
  size_t GetBlocksSize() { return blocks_.size(); }
  // Allocate()函数用于提供给上层调用者申请一块大小为bytes的内存
  // 不保证给上层调用者的内存首地址是字节对齐的
  char* Allocate(size_t bytes);
  // 如果上层调用者对 内存的字节对齐有要求，使用AllocateAligned()函数
  char* AllocateAligned(size_t bytes);
  // 统计内存使用量
  size_t MemoryUsage() const;

 private:
  // Allocate()的辅助函数
  char* AllocateFallback(size_t bytes);
  // 分配一个新的block
  char* AllocateNewBlock(size_t block_bytes);

 private:
  // alloc_ptr_指向的是当前可提供给上层调用者申请的内存的首地址
  char* alloc_ptr_;
  size_t alloc_bytes_remaining_;
  // blocks_来管理申请的内存块
  std::vector<char*> blocks_;
  // memory_usage_用于统计内存使用量
  std::atomic<size_t> memory_usage_;
};