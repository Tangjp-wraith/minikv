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

#include "../include/SkipList.h"

#include <iostream>
#include <ostream>

int main() {
  SkipList<int, std::string> mySkipList(6);
  mySkipList.insertElement(1, "sing");
  mySkipList.insertElement(2, "dance");
  mySkipList.insertElement(3, "rap");
  mySkipList.insertElement(4, "play basketball");
  mySkipList.insertElement(5, "cxk");

  mySkipList.displayList();

  mySkipList.insertElement(5, "cxk is best");

  std::cout << "SkipList's size: " << mySkipList.size() << std::endl;

  mySkipList.dumpFile();

  std::string value;
  mySkipList.searchElement(1, value);
  mySkipList.searchElement(4, value);

  mySkipList.deleteElement(1);
  mySkipList.deleteElement(4);

  std::cout << "SkipList's size : " << mySkipList.size() << std::endl;

  mySkipList.displayList();
  return 0;
}