/**
 * @file LRU.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef LRU_H
#define LRU_H
#include <iostream>
#include <list>
#include <unordered_map>

using namespace std;

template <typename K, typename V>
class LRU {
 private:
  int _capacity;
  // the list of LRU item, the front is the item most recently used
  list<pair<K, V>> _lst;
  unordered_map<K, typename list<pair<K, V>>::iterator> _mp;

 public:
  LRU(int c) : _capacity(c) { cout << "LRU build" << endl; };
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
    pair<K, V> p = *(_mp[k]);
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
  cout << "-------------LRUCache Begin--------------------" << endl;
  for (const auto& p : _lst) {
    cout << "key: " << p.first << ", value : " << p.second << endl;
  }
  cout << "--------------LRUCache End---------------------" << endl;
}
#endif