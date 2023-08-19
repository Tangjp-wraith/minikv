#include "arena.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>

static const int kBlockSize = 4096;


size_t Arena::MemoryUsage() const {
  return memory_usage_.load(std::memory_order_relaxed);
}

/*
分配时有如下三个规则:
  1.如果bytes小于当前块剩余内存大小，直接分配，并调整 alloc_ptr_ 与
    alloc_bytes_remaining_
  2.如果bytes大于当前块剩余内存大小，且小于1024(kBlockSize/ 4)，
    则申请一个新的块(block)，插入到 blocks_ 中，将 alloc_ptr_ 指向新块，调整
    alloc_bytes_remaining_ 为 kBlockSizebytes
  3.如果bytes大于1024(kBlockSize / 4)，那么直接分配一个新的块(block)，并插入
    blocks 中，此时 alloc_ptr_ 与 alloc_bytes_remaining_ 不做调整
*/
char* Arena::Allocate(size_t bytes) {
  // assert(bytes > 0);
  // 第一个规则
  if (bytes <= alloc_bytes_remaining_) {
    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
  }
  // 其他规则
  return AllocateFallback(bytes);
}

char* Arena::AllocateFallback(size_t bytes) {
  // 第三个规则
  if (bytes > kBlockSize / 4) {
    char* result = AllocateNewBlock(bytes);
    return result;
  }
  // 第二个规则
  alloc_ptr_ = AllocateNewBlock(kBlockSize);
  alloc_bytes_remaining_ = kBlockSize;
  char* result = alloc_ptr_;
  alloc_ptr_ += bytes;
  alloc_bytes_remaining_ -= bytes;
  return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
  char* result = new char[block_bytes];
  blocks_.push_back(result);
  //当前内存总共使用了之前的内存+当前分配内存块大小+指向当前内存块指针大小
  memory_usage_.fetch_add(block_bytes + sizeof(char*),
                          std::memory_order_relaxed);
  return result;
}

char* Arena::AllocateAligned(size_t bytes) {
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  static_assert((align & (align - 1)) == 0,
                "Pointer size should be a power of 2!");
  // x & (y - 1) = x%y => alloc_ptr_ % align
  // 计算alloc_ptr_指向的内存首地址相对于系统字节对齐的偏移量
  size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
  // 分配当前内存所需要填充的字节 slop
  size_t slop = (current_mod == 0 ? 0 : align - current_mod);
  // 计算总共分配的字节
  size_t needed = bytes + slop;

  char* result;
  if (needed <= alloc_bytes_remaining_) {
    result = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {
    result = AllocateFallback(bytes);
  }
  assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
  return result;
}
