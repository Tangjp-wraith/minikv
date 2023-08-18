#pragma once
#include <cstddef>
#include <iostream>
#include <vector>

class BitMap {
 public:
  BitMap() : _size(0) {}

  BitMap(size_t size) : _size(0) { _array.resize((size >> 5) + 1); }

  ~BitMap() = default;

  bool Set(size_t num) {
    size_t index = num >> 5;
    size_t n = num % 32;
    if (_array[index] & (1 << (31 - n))) {
      return false;
    } else {
      size_t a = (1 << (31 - n));
      _array[index] |= a;
      ++_size;
      return true;
    }
  }

  bool Found(size_t num) {
    size_t index = num >> 5;
    size_t n = num % 32;
    if (_array[index] & (1 << (31 - n))) {
      return true;
    }
    return false;
  }

  std::vector<size_t> _array;
  size_t _size;
};

template <class T>
size_t BKDRHash(const char* str) {
  size_t hash = 0;
  while (size_t ch = (size_t)*str++) {
    hash = hash * 131 + ch;
  }
  return hash;
}

template <class T>
size_t SDBMHash(const char* str) {
  size_t hash = 0;
  while (size_t ch = (size_t)*str++) {
    hash = 65599 * hash + ch;
  }
  return hash;
}

template <class T>
size_t RSHash(const char* str) {
  size_t hash = 0;
  size_t magic = 63689;
  while (size_t ch = (size_t)*str++) {
    hash = hash * magic + ch;
    magic *= 378551;
  }
  return hash;
}

template <class T>
size_t APHash(const char* str) {
  size_t hash = 0;
  size_t ch;
  for (long i = 0; (ch = (size_t)*str++); i++) {
    if ((i & 1) == 0) {
      hash ^= ((hash << 7) ^ ch ^ (hash >> 3));
    } else {
      hash ^= (~((hash << 11) ^ ch ^ (hash >> 5)));
    }
  }
  return hash;
}

template <class T>
size_t JSHash(const char* str) {
  if (!*str) {
    return 0;
  }
  size_t hash = 1315423911;
  while (size_t ch = (size_t)*str++) {
    hash ^= ((hash << 5) + ch + (hash >> 2));
  }
  return hash;
}

template <class T>
struct __HashFun1 {
  size_t operator()(const T& key) const { return BKDRHash<T>(key.c_str()); }
};

template <class T>
struct __HashFun2 {
  size_t operator()(const T& key) const { return SDBMHash<T>(key.c_str()); }
};

template <class T>
struct __HashFun3 {
  size_t operator()(const T& key) const { return RSHash<T>(key.c_str()); }
};

template <class T>
struct __HashFun4 {
  size_t operator()(const T& key) const { return APHash<T>(key.c_str()); }
};

template <class T>
struct __HashFun5 {
  size_t operator()(const T& key) const { return JSHash<T>(key.c_str()); }
};

template <class K = std::string, class HashFun1 = __HashFun1<K>,
          class HashFun2 = __HashFun2<K>, class HashFun3 = __HashFun3<K>,
          class HashFun4 = __HashFun4<K>, class HashFun5 = __HashFun5<K>>
class BloomFilter {
 public:
  BloomFilter(size_t size = 500000) : _capacity(size) {
    _bitmap._array.resize((size >> 5) + 1);
  }

  ~BloomFilter() = default;

  void _Set(const K& key) {
    _bitmap.Set(HashFun1()(key) % _capacity);
    _bitmap.Set(HashFun2()(key) % _capacity);
    _bitmap.Set(HashFun3()(key) % _capacity);
    _bitmap.Set(HashFun4()(key) % _capacity);
    _bitmap.Set(HashFun5()(key) % _capacity);
  }

  bool _IsIn(const K& key) {
    if (!_bitmap.Found(HashFun1()(key) % _capacity)) return false;
    if (!_bitmap.Found(HashFun1()(key) % _capacity)) return false;
    if (!_bitmap.Found(HashFun1()(key) % _capacity)) return false;
    if (!_bitmap.Found(HashFun1()(key) % _capacity)) return false;
    if (!_bitmap.Found(HashFun1()(key) % _capacity)) return false;
    return true;
  }

 private:
  BitMap _bitmap;
  size_t _capacity;
};
