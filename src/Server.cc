/**
 * @file Server.cc
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "../include/Server.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

#define DEBUG

#define BUFFERSIZE 1024
#define LEVELOFSKIPLIST 12

SkipList<std::string, std::string> SkipListWorker::mydb_(LEVELOFSKIPLIST);
ThreadPool SkipListWorker::read_threadpool_(2);

std::unordered_map<std::string, parseState> SkipListWorker::cmd_map_ = {
    {"SET", parseState::SET},
    {"GET", parseState::GET},
    {"DEL", parseState::DEL},
    {"LOAD", parseState::LOAD},
    {"DUMP", parseState::DUMP}};

std::vector<std::function<std::string(std::vector<std::string>&)>>
    SkipListWorker::handler_ = {
        {SkipListWorker::handlerERR},  {SkipListWorker::handlerSET},
        {SkipListWorker::handlerGET},  {SkipListWorker::handlerDEL},
        {SkipListWorker::handlerLOAD}, {SkipListWorker::handlerDUMP}};

std::string SkipListWorker::handlerERR(std::vector<std::string>& cmdList) {
#ifdef DEBUG

  std::cout << "handlerERR : " << std::endl;
  for (auto& str : cmdList) {
    std::cout << str << " ";
  }
  std::cout << std::endl;

#endif
  return "error : please enter a correct command : SET/DEL/GET/LOAD/DUMP\n";
}

std::string SkipListWorker::handlerSET(std::vector<std::string>& cmdList) {
  // _myDB.insertElement();
  // SET key value
  if (cmdList.size() != 3) {
    return "error : please enter a correct command : SET key value\n";
  }
  if (!mydb_.insertElement(cmdList[1], cmdList[2])) {
    return "ok : SET successfully\n";
  } else {
    return "warning : " + cmdList[1] + " is already existed\n";
  }
}

std::string SkipListWorker::handlerDEL(std::vector<std::string>& cmdList) {
  // DEL key
  if (cmdList.size() != 2) {
    return "error : please enter a correct command : DEL key\n";
  }
  if (mydb_.deleteElement(cmdList[1])) {
    return "ok : DEL successfully\n";
  } else {
    return "warning : " + cmdList[1] + " is not existed\n";
  }
}

std::string SkipListWorker::handlerGET(std::vector<std::string>& cmdList) {
  // GET key
  if (cmdList.size() != 2) {
    return "please enter a correct command : GET key\n";
  }
  std::string value;
  bool searchResult =
      read_threadpool_
          .enqueue([&]() { return mydb_.searchElement(cmdList[1], value); })
          .get();

  if (searchResult) {
    return "ok : " + value + "\n";
  } else {
    return "warning : " + cmdList[1] + " is not existed\n";
  }
}

std::string SkipListWorker::handlerDUMP(std::vector<std::string>& cmdList) {
  mydb_.dumpFile();
  return "ok : dump file successfully\n";
}

std::string SkipListWorker::handlerLOAD(std::vector<std::string>& cmdList) {
  mydb_.loadFile();
  return "ok : load file successfully\n";
}

void SkipListWorker::threadFunc(int acceptFd) {
  std::cout << "enter threadFunc now" << std::endl;
  char msgBuff[BUFFERSIZE];
  bzero(msgBuff, sizeof(msgBuff));
  while (recv(acceptFd, msgBuff, sizeof(msgBuff), 0) > 0) {
    std::string cmd(msgBuff);
    bzero(msgBuff, sizeof(msgBuff));
    std::string res = dealWithCmd(cmd);
#ifdef DEBUG

    std::cout << "res is '" << res << "'" << std::endl;

#endif
    send(acceptFd, res.c_str(), res.size(), 0);
  }
  close(acceptFd);
  std::cout << "connection closed" << std::endl;
}

parseState SkipListWorker::parseCommand(std::string cmd,
                                        std::vector<std::string>& cmdList) {
  std::string res;
  int left = 0, right = 0;
  while (right < cmd.size()) {
    while (right < cmd.size() && cmd[right] == ' ') {
      ++right;
    }
    left = right;
    while (right < cmd.size() && cmd[right] != ' ') {
      ++right;
    }
    std::string&& str = cmd.substr(left, right - left);
    if (str.size()) {
      cmdList.emplace_back(str);
    }
    ++right;
  }
  std::string cmdName = cmdList.front();
  std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), toupper);
  if (!cmd_map_.count(cmdName)) {
    std::cout << "cmdName do not exist..." << std::endl;
    return parseState::ERR;
  }

#ifdef DEBUG

  std::cout << "cmdList in parseCommand(2) is : " << std::endl;
  for (auto& str : cmdList) {
    std::cout << str << " ";
  }
  std::cout << std::endl;

#endif

  std::cout << "cmdName in parseCommand(2) is '" << cmdName << "'" << std::endl;
  return cmd_map_[cmdName];
}

std::string SkipListWorker::dealWithCmd(std::string cmd) {
  std::vector<std::string> cmdList;
  parseState state = SkipListWorker::parseCommand(cmd, cmdList);
  return handler_[state](cmdList);
}
