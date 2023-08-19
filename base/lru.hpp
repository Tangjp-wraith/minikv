#pragma once
#include <iostream>
#include <list>
#include <unordered_map>

template <typename K, typename V>
class LRU {
 private:
  int capacity_;
  // the list of LRU item, the front is the item most recently used
  std::list<std::pair<K, V>> lst_;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> mp_;

 public:
  LRU(int c) : capacity_(c) { std::cout << "LRU build" << std::endl; };
  ~LRU() = default;
  bool get(K, V&);
  void put(K, V);
  void del(K);
  bool is_find(K);
  void printLRUCache();
};

// search the key in the LRU block, if found, move to the front
template <typename K, typename V>
bool LRU<K, V>::get(K k, V& v) {
  if (mp_.find(k) != mp_.end()) {
    std::pair<K, V> p = *(mp_[k]);
    v = p.second;
    lst_.erase(mp_[k]);
    lst_.push_front(p);
    mp_[k] = lst_.begin();
    return true;
  }
  return false;
}

// insert the key item
template <typename K, typename V>
void LRU<K, V>::put(K k, V v) {
  if (mp_.find(k) != mp_.end()) {
    lst_.erase(mp_[k]);
    lst_.push_front({k, v});
    mp_[k] = lst_.begin();
  } else {
    if (lst_.size() == capacity_) {
      auto tail = lst_.back();
      lst_.pop_back();
      mp_.erase(tail.first);
      lst_.push_front({k, v});
      mp_[k] = lst_.begin();
    } else {
      lst_.push_front({k, v});
      mp_[k] = lst_.begin();
    }
  }
}

// delete the key item
template <typename K, typename V>
void LRU<K, V>::del(K k) {
  if (mp_.find(k) == mp_.end()) return;
  lst_.erase(mp_[k]);
  mp_.erase(k);
}

// find the key item or not
template <typename K, typename V>
bool LRU<K, V>::is_find(K k) {
  return mp_.find(k) != mp_.end();
}

template <typename K, typename V>
void LRU<K, V>::printLRUCache() {
  std::cout << "-------------LRUCache Begin--------------------" << std::endl;
  for (const auto& p : lst_) {
    std::cout << "key: " << p.first << ", value : " << p.second << std::endl;
  }
  std::cout << "--------------LRUCache End---------------------" << std::endl;
}

