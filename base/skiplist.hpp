#pragma once

#include <atomic>
#include <cassert>
#include <string>
#include <vector>

#include "arena.h"
#include "random.h"

class Arena;

template <typename Key, typename Value, class Comparator>
class SkipList {
 private:
  struct Node;

 public:
  explicit SkipList(Comparator cmp, Arena* arena);

  SkipList(const SkipList&) = delete;
  SkipList& operator=(const SkipList&) = delete;

 private:
  enum { kMaxHeight = 12 };

  Node* NewNode(const Key& key, const Value& value, int height);

  Comparator compare_;
  Arena* arena_;
  Node* head_;
  std::atomic<int> max_height_;
  Random rnd_;
};

template <typename Key, typename Value, class Comparator>
struct SkipList<Key, Value, Comparator>::Node {
  explicit Node(const Key& key, const Value& value)
      : key_(key), value_(value) {}

  Node* Next(int n) {
    assert(n >= 0);
    return next_[n].load(std::memory_order_acquire);
  }

  void SetNext(int n, Node* x) {
    assert(n >= 0);
    return next_[n].store(x, std::memory_order_release);
  }

  Node* NoBarrier_Next(int n) {
    assert(n >= 0);
    return next_[n].load(std::memory_order_relaxed);
  }

  void NoBarrier_SetNext(int n, Node* x) {
    assert(n >= 0);
    next_[n].store(x, std::memory_order_relaxed);
  }

 private:
  Key key_;
  Value value_;
  std::atomic<Node*> next_[1];
};

template <typename Key, typename Value, class Comparator>
SkipList<Key, Value, Comparator>::SkipList(Comparator cmp, Arena* arena)
    : compare_(cmp),
      arena_(arena),
      head_(NewNode(0, {}, kMaxHeight)),
      max_height_(1),
      rnd_(0xdeadbeef) {
  for (int i = 0; i < kMaxHeight; ++i) {
    head_->SetNext(i, nullptr);
  }
}

/*
  NewNode()方法用于申请并初始化一个节点,分配12层内存
  默认有一层，总共12层，那么需要再动态分配 height-1层
  而每个节点占用sizeof(std::atomic<Node*>)大小，
  所以总共需要sizeof(Node)+sizeof(std::atomic<Node*>)*(height-1))个大小
  sizeof(Node)=>当前节点整个node大小
  sizeof(std::atomic<Node*>)*(height-1))=>剩余的节点大小
*/
template <typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node*
SkipList<Key, Value, Comparator>::NewNode(const Key& key, const Value& value,
                                          int height) {
  char* node_memory = arena_->AllocateAligned(
      sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
  return new (node_memory) Node(key, value);
}

