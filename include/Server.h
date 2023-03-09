/**
 * @file Server.h
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-08
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "SkipList.h"
#include "ThreadPool.h"

enum parseState { ERR = 0, SET = 1, GET = 2, DEL = 3, LOAD = 4, DUMP = 5 };

class SkipListWorker {
 public:
  static void threadFunc(int acceptFd);
  static parseState parseCommand(std::string cmd,
                                 std::vector<std::string>& cmdList);
  static std::string dealWithCmd(std::string cmd);
  static SkipList<std::string, std::string> mydb_;

 private:
  static std::unordered_map<std::string, parseState> cmd_map_;
  static std::vector<std::function<std::string(std::vector<std::string>&)>>
      handler_;
  static ThreadPool read_threadpool_;
  static std::string handlerERR(std::vector<std::string>& cmdList);
  static std::string handlerSET(std::vector<std::string>& cmdList);
  static std::string handlerGET(std::vector<std::string>& cmdList);
  static std::string handlerDEL(std::vector<std::string>& cmdList);
  static std::string handlerLOAD(std::vector<std::string>& cmdList);
  static std::string handlerDUMP(std::vector<std::string>& cmdList);
};