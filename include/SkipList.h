/**
 * @file skiplist.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <string>

#include "Node.h"
#include "Random.h"

const std::string STORE_FILE = "../store/dumpFile";
const std::string DELIMITER = ":";

static std::mutex mtx;

template <typename K, typename V>
class SkipList {
 public:
  SkipList();
  SkipList(int max_level);
  ~SkipList();

  Node<K, V> *createNode(const K k, const V v, int level);
  void displayList();
  bool insertElement(const K key, const V value);
  bool searchElement(K key, V &value);
  bool deleteElement(K key);

  int size();

  void dumpFile();
  void loadFile();

 private:
  void getKeyValueFromString(const std::string &str, std::string *key,
                             std::string *value);
  bool isValidString(const std::string &str);
  int getRandomLevel();

 private:
  /// @brief 跳表的最大层数
  int max_level_;
  /// @brief 当前所在层数
  int cur_level_;
  /// @brief 跳表头节点指针
  Node<K, V> *header_;
  /// @brief 当前元素个数
  int element_count_;
  /// @brief 文件操作 写
  std::ofstream file_writer_;
  /// @brief 文件操作 读
  std::ifstream file_reader_;

  Random rnd_;
};

template <typename K, typename V>
SkipList<K, V>::SkipList()
    : max_level_(32), cur_level_(0), element_count_(0), rnd_(1) {
  K k;
  V v;
  // 创建头节点 并将K,V初始化为NULL
  header_ = new Node<K, V>(k, v, max_level_);
  loadFile();
}

// 建造跳表
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level)
    : max_level_(max_level), cur_level_(0), element_count_(0), rnd_(1) {
  K k;
  V v;
  header_ = new Node<K, V>(k, v, max_level_);
}

template <typename K, typename V>
SkipList<K, V>::~SkipList() {
  if (file_writer_.is_open()) {
    file_writer_.close();
  }
  if (file_reader_.is_open()) {
    file_reader_.close();
  }
  delete header_;
}

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::createNode(const K k, const V v, int level) {
  Node<K, V> *node = new Node<K, V>(k, v, level);
  return node;
}

// 打印整个跳表
template <typename K, typename V>
void SkipList<K, V>::displayList() {
#ifdef DEBUG
  std::cout << "display SkipList : " << std::endl;
#endif

  Node<K, V> *cur;
  for (int i = cur_level_; i >= 0; --i) {
    cur = header_->forward_[i];
#ifdef DEBUG
    std::cout << "Level : " << i << ":";
#endif
    while (cur != nullptr) {
#ifdef DEBUG
      std::cout << cur->getKey() << ":" << cur->getValue() << " ";
#endif
      cur = cur->forward_[i];
    }
#ifdef DEBUG
    std::cout << std::endl;
#endif
  }
  return;
}

// 向跳表中插入节点
template <typename K, typename V>
bool SkipList<K, V>::insertElement(const K key, const V value) {
  mtx.lock();
  Node<K, V> *cur = header_;
  // 创建数组update并初始化
  Node<K, V> **update = new Node<K, V> *[max_level_ + 1]();
  // 从跳表的左上角节点开始查找
  for (int i = cur_level_; i >= 0; --i) {
    // cur在该层的下一个节点的key_小于要找的key
    while (cur->forward_[i] != nullptr && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
    update[i] = cur;
  }
  //到达最底层，并且当前forward指针指向第一个大于待插入节点的节点
  cur = cur->forward_[0];
  // 如果当前节点的key与待插入节点的key相等，直接修改节点值即可
  if (cur != nullptr && cur->getKey() == key) {
#ifdef DEBUG
    std::cout << "key: " << key << ", exists. Change it's value" << std::endl;
#endif
    cur->setValue(value);
    mtx.unlock();
    return false;
  }
  // 如果current节点为null 这就说明要将该元素插入到最后一个节点
  // 如果current节点的key值和待插入的key不等
  // 需要在update[0]和current之间插入该节点
  if (cur == nullptr || cur->getKey() != key) {
    int randomLevel = getRandomLevel();
    if (randomLevel > cur_level_) {
      for (int i = cur_level_ + 1; i <= randomLevel; ++i) {
        update[i] = header_;
      }
      cur_level_ = randomLevel;
    }
    // 使用生成的random level 创建新节点
    Node<K, V> *insertNode = createNode(key, value, randomLevel);
    // 插入节点
    for (int i = 0; i <= randomLevel; ++i) {
      insertNode->forward_[i] = update[i]->forward_[i];
      update[i]->forward_[i] = insertNode;
    }
#ifdef DEBUG
    std::cout << "Successfully inserted key: " << key << ", value:" << value
              << std::endl;
#endif
    ++element_count_;
  }
  mtx.unlock();
  return true;
}

// 从条表中查询元素
template <typename K, typename V>
bool SkipList<K, V>::searchElement(K key, V &value) {
#ifdef DEBUG
  std::cout << "-----------search element----------" << std::endl;
#endif
  Node<K, V> *cur = header_;
  for (int i = cur_level_; i >= 0; --i) {
    while (cur->forward_[i] && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
  }
  cur = cur->forward_[0];
  if (cur != nullptr && cur->getKey() == key) {
    value = cur->getValue();
#ifdef DEBUG
    std::cout << "Found key:" << key << ", value" << cur->getValue()
              << std::endl;
#endif
    return true;
  }
  return false;
}

// 从跳表中删除元素
template <typename K, typename V>
bool SkipList<K, V>::deleteElement(K key) {
  mtx.lock();
  Node<K, V> **update = new Node<K, V> *[max_level_ + 1]();
  Node<K, V> *cur = header_;
  for (int i = cur_level_; i >= 0; --i) {
    while (cur->forward_[i] && cur->forward_[i]->getKey() < key) {
      cur = cur->forward_[i];
    }
    update[i] = cur;
  }
  cur = cur->forward_[0];
  if (cur != nullptr && cur->getKey() == key) {
    // 从最底层开始 删除每一层的current节点
    for (int i = 0; i <= cur_level_; ++i) {
      if (update[i]->forward_[i] != cur) {
        break;
      }
      update[i]->forward_[i] = cur->forward_[i];
    }
    delete cur;
    // 当前层为空 则删除层
    while (cur_level_ > 0 && header_->forward_[cur_level_] == nullptr) {
      --cur_level_;
    }
    --element_count_;
    mtx.unlock();
    return true;
  } else {
    mtx.unlock();
    return false;
  }
}

template <typename K, typename V>
int SkipList<K, V>::size() {
  return element_count_;
}

// 将数据导出到文件
template <typename K, typename V>
void SkipList<K, V>::dumpFile() {
#ifdef DEBUG
  std::cout << "-----------dumpfile----------" << std::endl;
#endif
  if (!file_writer_.is_open()) {
    file_writer_.open(STORE_FILE, std::ios::out | std::ios::trunc);
  }
  Node<K, V> *cur = header_->forward_[0];
  while (cur != nullptr) {
    file_writer_ << cur->getKey() << ":" << cur->getValue() << "\n";
#ifdef DEBUG
    std::cout << cur->getKey() << ":" << cur->getValue() << std::endl;
#endif
    cur = cur->forward_[0];
  }
  file_writer_.flush();
  file_writer_.close();
  return;
}

// 从磁盘读取数据
template <typename K, typename V>
void SkipList<K, V>::loadFile() {
#ifdef DEBUG
  std::cout << "-----------loadfile----------" << std::endl;
#endif
  file_reader_.open(STORE_FILE);
  std::string line;
  std::string *key = new std::string();
  std::string *value = new std::string();
  while (getline(file_reader_, line)) {
    getKeyValueFromString(line, key, value);
    if (key->empty() || value->empty()) {
      continue;
    }
    insertElement(*key, *value);
#ifdef DEBUG
    std::cout << "key: " << *key << " value: " << value << std::endl;
#endif
  }
  file_reader_.close();
}

template <typename K, typename V>
void SkipList<K, V>::getKeyValueFromString(const std::string &str,
                                           std::string *key,
                                           std::string *value) {
  if (!isValidString(str)) {
    return;
  }
  int pos = str.find(DELIMITER);
  *key = str.substr(0, pos);
  *value = str.substr(pos + 1, str.size());
}

template <typename K, typename V>
bool SkipList<K, V>::isValidString(const std::string &str) {
  if (str.empty() || str.find(DELIMITER) == std::string::npos) {
    return false;
  }
  return true;
}

template <typename K, typename V>
int SkipList<K, V>::getRandomLevel() {
  int level = static_cast<int>(rnd_.Uniform(max_level_));
  if (level == 0) {
    level = 1;
  }
  return level;
}
