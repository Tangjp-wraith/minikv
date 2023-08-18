#pragma once
#include <iostream>
#include <list>
#include <unordered_map>

template <typename K, typename V>
class LRU {
 private:
  int _capacity;
  // the list of LRU item, the front is the item most recently used
  std::list<std::pair<K, V>> _lst;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> _mp;

 public:
  LRU(int c) : _capacity(c) { std::cout << "LRU build" << std::endl; };
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
  if (_mp.find(k) != _mp.end()) {
    std::pair<K, V> p = *(_mp[k]);
    v = p.second;
    _lst.erase(_mp[k]);
    _lst.push_front(p);
    _mp[k] = _lst.begin();
    return true;
  }
  return false;
}

// insert the key item
template <typename K, typename V>
void LRU<K, V>::put(K k, V v) {
  if (_mp.find(k) != _mp.end()) {
    _lst.erase(_mp[k]);
    _lst.push_front({k, v});
    _mp[k] = _lst.begin();
  } else {
    if (_lst.size() == _capacity) {
      auto tail = _lst.back();
      _lst.pop_back();
      _mp.erase(tail.first);
      _lst.push_front({k, v});
      _mp[k] = _lst.begin();
    } else {
      _lst.push_front({k, v});
      _mp[k] = _lst.begin();
    }
  }
}

// delete the key item
template <typename K, typename V>
void LRU<K, V>::del(K k) {
  if (_mp.find(k) == _mp.end()) return;
  _lst.erase(_mp[k]);
  _mp.erase(k);
}

// find the key item or not
template <typename K, typename V>
bool LRU<K, V>::is_find(K k) {
  return _mp.find(k) != _mp.end();
}

template <typename K, typename V>
void LRU<K, V>::printLRUCache() {
  std::cout << "-------------LRUCache Begin--------------------" << std::endl;
  for (const auto& p : _lst) {
    std::cout << "key: " << p.first << ", value : " << p.second << std::endl;
  }
  std::cout << "--------------LRUCache End---------------------" << std::endl;
}

