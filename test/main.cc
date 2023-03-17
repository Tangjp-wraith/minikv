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

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>

#include "../include/SkipList.h"

std::string randStr(int len) {
  std::string ans = "";
  for (int i = 0; i < len; i++) {
    ans += ('a' + rand() % 26);
  }
  return ans;
}

int main() {
  SkipList<std::string, std::string> mykv(12);
  // mykv.loadFile();
  for (int i = 0; i < 100; i++) {
    mykv.insertElement(randStr(2), randStr(rand() % 10));
  }

  for (int i = 0; i < 10; i++) {
    mykv.deleteElement(randStr(2));
  }

  for (int i = 0; i < 100; i++) {
    mykv.element_expire_time(randStr(2), rand() % 10);
  }

  for (int i = 0; i < 100; i++) {
    mykv.element_ttl(randStr(2));
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