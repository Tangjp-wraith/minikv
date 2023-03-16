/**
 * @file SkipList_test.cc
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <unistd.h>

#include <iostream>
#include <ostream>

#include "../include/SkipList.h"

std::string randStr(int len) {
  std::string ans = "";
  for (int i = 0; i < len; i++) {
    ans += ('a' + rand() % 26);
  }
  return ans;
}

int main() {
  SkipList<int, std::string> mykv(12);
  // mykv.loadFile();
  for (int i = 0; i < 100; i++) {
    mykv.insertElement(i, randStr(rand() % 10000));
  }

  // for (int i = 0; i < 10; i++) {
  //   mykv.deleteElement(i);
  // }

  for (int i = 0; i < 100; i++) {
    mykv.element_expire_time(i, rand() % 10);
  }

  for (int i = 0; i < 100; i++) {
    mykv.element_ttl(i);
  }
  sleep(5);
  // mykv.cycle_del();

  // for (int i = 0; i < 100; i++) {
  //   mykv.deleteElement(rand()%100);
  // }

  // mykv.displayList();

  mykv.dumpFile();
  return 0;
}