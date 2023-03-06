/**
 * @file Node.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

template <typename K, typename V>
class SkipList;

template <typename K, typename V>
class Node {
  friend class SkipList<K, V>;

 public:
  Node() {}
  Node(K k, V v, int level);
  ~Node();

  K getKey() const;
  V getValue() const;
  void setValue(V value);

  // 不同层数的下一节点
  Node<K, V> **forward_;
  int node_level_;

 private:
  K key_;
  V value_;
};

template <typename K, typename V>
inline Node<K, V>::Node(K k, V v, int level)
    : key_(k), value_(v), node_level_(level) {
  forward_ = new Node<K, V> *[node_level_ + 1]();
}

template <typename K, typename V>
inline Node<K, V>::~Node() {
  delete[] forward_;
}

template <typename K, typename V>
inline K Node<K, V>::getKey() const {
  return key_;
}

template <typename K, typename V>
inline V Node<K, V>::getValue() const {
  return value_;
}

template <typename K, typename V>
inline void Node<K, V>::setValue(V value) {
  value_ = value;
}
