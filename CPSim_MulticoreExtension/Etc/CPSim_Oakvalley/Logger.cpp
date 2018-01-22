/**
  *@File    Logger.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   All of the prompt messages are printed here
  */
#include <cstdio>
#include <cstdlib>

#include "Logger.hpp"

std::vector<std::string> Logger::__log;

void Logger::logAndExit(
    std::string _log_id,
    std::string _log_msg,
    std::string _log_arg) {
  /* TODO : call exit of child threads */
  printf("CPSim is exited (%s) : %s %s\n", _log_id.c_str(), _log_msg.c_str(), _log_arg.c_str());
  exit(-1);
}

void Logger::appendLog(std::string _log) {
  __log.push_back(_log);
}

void Logger::clearLog(void) {
  __log.clear();
}

void Logger::printLog(void) {
  printf("CPSim prints log\n");
  for (auto it = __log.begin(); it != __log.end(); ++it) {
    printf("%s\n", it->c_str());
  }

  clearLog();
}
