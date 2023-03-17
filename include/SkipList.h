/**
 * @file SkipList.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <time.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "BloomFilter.h"
#include "LRU.h"

#define STORE_FILE "../store/dumpFile.txt"
#define LRU_DEFAULT_SIZE 8
#define CYCLE_DEL_NUM 20

static std::mutex mtx;

template <typename T>
class Less {
 public:
  bool operator()(const T& a, const T& b) const { return a < b; }
};

template <typename K, typename V>
class Node {
 public:
  Node() = default;
  Node(K k, V v, int level);
  ~Node();

  K getKey() const { return _key; };
  V getValue() const { return _value; };
  void setValue(V v) { _value = v; };

  Node<K, V>** _forward;
  int _nodeLevel;

 private:
  K _key;
  V _value;
};

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
  _key = k;
  _value = v;
  _nodeLevel = level;
  _forward = new Node<K, V>*[_nodeLevel + 1]();
}

template <typename K, typename V>
Node<K, V>::~Node() {
  delete[] _forward;
}

template <typename K, typename V, typename Comp = Less<K>>
class SkipList {
 public:
  SkipList() = default;
  SkipList(int level, int lrusize = LRU_DEFAULT_SIZE);
  ~SkipList();

  int getRandomLevel();
  Node<K, V>* createNode(K, V, int);
  void displayList();
  int insertElement(K, V);
  bool searchElement(K, V&);
  bool deleteElement(K);
  int size() { return _elementCount; };

  void dumpFile();
  void loadFile();

  void element_expire_time(K, int);
  int element_ttl(const K);
  void cycle_del();

  void printLRU() { _lrulist->printLRUCache(); };

 private:
  void get_key_value_from_string(const std::string& str, std::string* key,
                                 std::string* value);
  bool is_valid_string(const std::string& str);
  int is_expire(K k);

 private:
  // max level of the skip list
  int _maxLevel;
  // curlevel of the skip list
  int _curLevel;
  // head ptr
  Node<K, V>* _header;

  BloomFilter<K> BF;
  // cur element num
  int _elementCount;
  // file operator
  std::ofstream _fileWriter;
  std::ifstream _fileReader;

  // the store timestamp and time of alive
  std::unordered_map<K, std::pair<int, time_t>> expire_key_mp;
  // LRU cache
  LRU<K, V>* _lrulist;

  Comp _less;
};

// init of SkipList
template <typename K, typename V, typename Comp>
SkipList<K, V, Comp>::SkipList(int level, int lrusize) {
  _maxLevel = level;
  _curLevel = 0;
  _elementCount = 0;
  K k;
  V v;
  _header = new Node<K, V>(k, v, _maxLevel);
  _lrulist = new LRU<K, V>(lrusize);
  // _less(Comp());
}

// destroy of SkipList
template <typename K, typename V, typename Comp>
SkipList<K, V, Comp>::~SkipList() {
  if (_fileWriter.is_open()) _fileWriter.close();
  if (_fileReader.is_open()) _fileReader.close();
  delete _header;
  delete _lrulist;
}

// random level of the node
template <typename K, typename V, typename Comp>
int SkipList<K, V, Comp>::getRandomLevel() {
  int k = 0;
  while (rand() % 2) k++;
  k = (k < _maxLevel) ? k : _maxLevel;
  return k;
}

// create Node<K,V>
template <typename K, typename V, typename Comp>
Node<K, V>* SkipList<K, V, Comp>::createNode(K k, V v, int level) {
  Node<K, V>* node = new Node<K, V>(k, v, level);
  return node;
}

// insert element
template <typename K, typename V, typename Comp>
int SkipList<K, V, Comp>::insertElement(K k, V v) {
  // lock
  mtx.lock();

  std::cout << "begin insert key: " << k << std::endl;
  // if the item is expired, else put the item in LRU
  if (is_expire(k) == 1) {
    std::cout << "expired, lazy delete the key: " << k << std::endl;
    deleteElement(k);
  } else {
    std::cout << "put the key: " << k << std::endl;
    _lrulist->put(k, v);
  }

  BF._Set(k);

  Node<K, V>* cur = _header;

  // track the parent of the inserted-Node
  Node<K, V>** update = new Node<K, V>*[_maxLevel + 1];
  for (int i = _curLevel; i >= 0; i--) {
    while (cur->_forward[i] && _less(cur->_forward[i]->getKey(), k))
      cur = cur->_forward[i];
    update[i] = cur;
  }

  // the position of key
  cur = cur->_forward[0];

  // the key is already in the skiplist, modify its value
  if (cur != nullptr &&
      (!_less(cur->getKey(), k) && !_less(k, cur->getKey()))) {
    // std::cout<<"modify the Node key: "<<k<<", value: "<<v<<std::endl;
    _lrulist->printLRUCache();
    cur->setValue(v);
    mtx.unlock();
    return 1;
  }

  // insert the new Node
  if (cur == nullptr || _less(k, cur->getKey())) {
    int randomLevel = getRandomLevel();
    // the new Node's level is higher than _curLevel
    if (randomLevel > _curLevel) {
      for (int i = _curLevel + 1; i <= randomLevel; i++) {
        update[i] = _header;
      }
      _curLevel = randomLevel;
    }
    // insert
    Node<K, V>* insertNode = createNode(k, v, randomLevel);
    for (int i = 0; i <= randomLevel; i++) {
      insertNode->_forward[i] = update[i]->_forward[i];
      update[i]->_forward[i] = insertNode;
    }
    std::cout << "Successfully inserted key: " << k << ", value: " << v
              << std::endl;
    _elementCount++;
  }

  // unlock the mutex
  mtx.unlock();
  return 0;
}

// search the given key, and return its value
template <typename K, typename V, typename Comp>
bool SkipList<K, V, Comp>::searchElement(K k, V& v) {
  if (!BF._IsIn(k)) {
    std::cout << "BloomFilter: key=" << k << " doesn't exist" << std::endl;
    return false;
  }

  // firstly search from LRU
  if (_lrulist->get(k, v)) {
    // lazy delete
    if (is_expire(k) == 1) {
      std::cout << "The key: " << k << " has expired, lazy delete it"
                << std::endl;
      deleteElement(k);
      return false;
    }
    _lrulist->put(k, v);
    // std::cout << "Found key: " << k << ", value: " << v << " and move to the
    // head of LRU"<<std::endl;
    return true;
  }
  Node<K, V>* cur = _header;
  for (int i = _curLevel; i >= 0; i--) {
    while (cur->_forward[i] && _less(cur->_forward[i]->getKey(), k))
      cur = cur->_forward[i];
  }
  cur = cur->_forward[0];
  // lazy delete
  if (cur && is_expire(cur->getKey()) == 1) {
    deleteElement(cur->getKey());
    return false;
  }
  // find the key-value
  if (cur != nullptr &&
      (!_less(cur->getKey(), k) && !_less(k, cur->getKey()))) {
    v = cur->getValue();
    _lrulist->put(k, v);
    // std::cout << "Found key: " << k << ", value: " << cur->getValue() <<" and
    // put into the LRU"<< std::endl;
    return true;
  }
  // std::cout << "Not Found Key:" << k << std::endl;
  return false;
}

// delete the given key element
template <typename K, typename V, typename Comp>
bool SkipList<K, V, Comp>::deleteElement(K k) {
  // lock the mutex
  mtx.lock();

  if (!BF._IsIn(k)) {
    std::cout << "BloomFilter: key=" << k << " doesn't exist" << std::endl;
    mtx.unlock();
    return false;
  }

  // whether the key in the LRU cache
  if (_lrulist->is_find(k)) {
    _lrulist->del(k);
    expire_key_mp.erase(k);
  }

  Node<K, V>* cur = _header;
  // track the parent
  Node<K, V>** update = new Node<K, V>*[_maxLevel + 1];
  for (int i = _curLevel; i >= 0; i--) {
    while (cur->_forward[i] && _less(cur->_forward[i]->getKey(), k))
      cur = cur->_forward[i];
    update[i] = cur;
  }
  cur = cur->_forward[0];

  // if find the key-element, delete
  if (cur != nullptr &&
      (!_less(cur->getKey(), k) && !_less(k, cur->getKey()))) {
    for (int i = 0; i <= _curLevel; i++) {
      if (update[i]->_forward[i] != cur) break;
      update[i]->_forward[i] = cur->_forward[i];
    }
    std::cout << "Delete key: " << k << " value: " << cur->getValue()
              << std::endl;
    delete cur;
    while (_curLevel > 0 && _header->_forward[_curLevel] == nullptr)
      _curLevel--;
    _elementCount--;
    mtx.unlock();
    return true;
  }
  std::cout << "Delete key: " << k << " failed, not exist" << std::endl;
  mtx.unlock();
  return false;
}

// display the skip list
// in level mode
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::displayList() {
  std::cout << "\n**********Display SkipList**********\n";
  Node<K, V>* cur;
  for (int i = _curLevel; i >= 0; i--) {
    cur = _header->_forward[i];
    std::cout << "Level " << i << std::endl;
    while (cur != nullptr) {
      std::cout << cur->getKey() << ":" << cur->getValue() << "; ";
      cur = cur->_forward[i];
    }
    std::cout << std::endl;
  }
  std::cout << "\n************Display  End************\n";
}

// write return disk
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::dumpFile() {
  std::cout << "\ndump file\n";
  _fileWriter.open(STORE_FILE);
  if (!_fileWriter.is_open()) {
    std::cout << "file not open" << std::endl;
    return;
  }
  Node<K, V>* cur = _header->_forward[0];
  while (cur != nullptr) {
    _fileWriter << cur->getKey() << ":" << cur->getValue() << std::endl;
    cur = cur->_forward[0];
  }
  _fileWriter.flush();
  _fileWriter.close();
}

// load the data from disk
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::loadFile() {
  std::cout << "\nload file\n";
  _fileReader.open(STORE_FILE);
  if (!_fileReader.is_open()) {
    std::cout << "file not open" << std::endl;
    return;
  }
  std::string line;
  std::string* key = new std::string();
  std::string* value = new std::string();
  while (getline(_fileReader, line)) {
    get_key_value_from_string(line, key, value);
    if (key->empty() || value->empty()) continue;
    insertElement(stoi(*key), *value);
    std::cout << "load item key: " << *key << " value: " << *value << std::endl;
  }
  _fileReader.close();
}

// recover the KV-item from string
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::get_key_value_from_string(const std::string& str,
                                                     std::string* key,
                                                     std::string* value) {
  if (str.empty() || str.find(':') == std::string::npos) return;
  *key = str.substr(0, str.find(':'));
  *value = str.substr(str.find(':') + 1);
}

// set the expire time of the key
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::element_expire_time(K k, int seconds) {
  V v;
  if (searchElement(k, v) == false) {
    std::cout << "expire time set failed, "
              << "key: " << k << " not found" << std::endl;
    return;
  }

  time_t tm;
  time(&tm);
  expire_key_mp[k] = std::make_pair(seconds, tm);
  std::cout << "successfully set the expire time of key: " << k << " seconds "
            << seconds << std::endl;
}

template <typename K, typename V, typename Comp>
int SkipList<K, V, Comp>::is_expire(K k) {
  // not found
  // std::cout<<"try to find key: "<<k<<" in LRU"<<std::endl;
  if (expire_key_mp.find(k) == expire_key_mp.end()) return -1;
  time_t tm;
  time(&tm);
  // is expire or not
  if (tm - expire_key_mp[k].second >= expire_key_mp[k].first)
    return 1;
  else
    return 0;
}

// return the ttl of the given key
template <typename K, typename V, typename Comp>
int SkipList<K, V, Comp>::element_ttl(const K k) {
  if (expire_key_mp.find(k) == expire_key_mp.end()) {
    std::cout << "ask for the ttl for a permanent key: " << k << std::endl;
    return -1;
  }

  if (is_expire(k)) {
    deleteElement(k);
    std::cout << "key: " << k << " is expired, delete it" << std::endl;
  }

  time_t tm;
  time(&tm);
  int sec = expire_key_mp[k].first - (tm - expire_key_mp[k].second);
  std::cout << "key: " << k << " has " << sec << " seconds left" << std::endl;
  return sec;
}

// cycle delete
template <typename K, typename V, typename Comp>
void SkipList<K, V, Comp>::cycle_del() {
  int cnt, num;
  do {
    cnt = 0;
    num = std::min(int(expire_key_mp.size()), CYCLE_DEL_NUM);
    int i = 0;
    std::vector<K> del_vec;
    for (auto& ele : expire_key_mp) {
      K key = ele.first;
      if (is_expire(key)) {
        del_vec.emplace_back(key);
        cnt++;
      }
      if (++i >= num) break;
    }
    for (auto k : del_vec) {
      std::cout << "Cycle delete, "
                << "key: " << k << std::endl;
      deleteElement(k);
    }
  } while (num * 0.5 < cnt);
}
#endif