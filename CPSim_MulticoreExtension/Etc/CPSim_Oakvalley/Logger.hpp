/**
  *@File    Logger.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   All of the prompt messages are printed here
  */
#ifndef __Logger__HPP__
#define __Logger__HPP__

#include <vector>
#include <string>

class Logger {
private:
  static std::vector<std::string> __log;

public:
  static void logAndExit(
      std::string _log_id,
      std::string _log_msg,
      std::string _log_arg);

  static void appendLog(std::string _log);

  static void clearLog(void);

  static void printLog(void);
};

#endif
