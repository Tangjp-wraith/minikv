/**
 * @file Command.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <string.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

const std::vector<std::string> cmdList = {"set", "get", "del", "load", "dump"};
const std::string DELIMITER = " ";

class Command {
 public:
  Command(std::string cmd) : command_(cmd) {}
  std::string getCommand();
  bool isValidCommand();
  void splitCommand();
  void printError(int error);
  std::vector<std::string> arg_;

 private:
  std::string command_;
};

inline std::string Command::getCommand() { return command_; }

inline bool Command::isValidCommand() {
  splitCommand();
  if (!arg_.size()) {
    return false;
  }
  std::string cmd = arg_[0];
  bool isValid = false;
  for (int i = 0; i < cmdList.size(); ++i) {
    if (cmd == cmdList[i]) {
      isValid = true;
    }
  }
  if (!isValid) {  // unknow cmd
    printError(1);
    return false;
  }
  if (cmd == cmdList[0] && arg_.size() != 3) {  // set key value
    printError(2);
    return false;
  } else if (cmd == cmdList[1] && arg_.size() != 2) {  // get key
    printError(3);
    return false;
  } else if (cmd == cmdList[2] && arg_.size() != 2) {  // del key
    printError(4);
    return false;
  } else if (cmd == cmdList[3] && arg_.size() != 1) {  // load
    printError(5);
    return false;
  } else if (cmd == cmdList[4] && arg_.size() != 1) {  // dump
    printError(6);
    return false;
  }
  return true;
}

inline void Command::splitCommand() {
  if (command_.empty()) {
    return;
  }
  std::string strs = getCommand() + DELIMITER;
  size_t pos = strs.find(DELIMITER);
  size_t size = strs.size();
  while (pos != std::string::npos) {
    std::string cmd = strs.substr(0, pos);
    std::cout << cmd << " ";
    arg_.emplace_back(cmd);
    strs = strs.substr(pos + 1, size);
    pos = strs.find(DELIMITER);
  }
  std::cout << std::endl;
  return;
}

inline void Command::printError(int error) {
  switch (error) {
    case 1 /* constant-expression */:
      std::cout << "(error) ERR unknown command" << std::endl;
      break;

    case 2 /* constant-expression */:
      std::cout << "(error) ERR wrong number of arguments for 'set' command"
                << std::endl;
      break;

    case 3 /* constant-expression */:
      std::cout << "(error) ERR wrong number of arguments for 'get' command"
                << std::endl;
      break;
    case 4 /* constant-expression */:
      std::cout << "(error) ERR wrong number of arguments for 'del' command"
                << std::endl;
      break;

    case 5 /* constant-expression */:
      std::cout << "(error) ERR wrong number of arguments for 'load' command"
                << std::endl;
      break;
    case 6 /* constant-expression */:
      std::cout << "(error) ERR wrong number of arguments for 'dump' command"
                << std::endl;
      break;
    default:
      break;
  }
}