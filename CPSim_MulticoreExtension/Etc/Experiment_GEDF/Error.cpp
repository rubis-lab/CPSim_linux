/**
  *@File    Error.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   All of the error messages are printed here
  */
#include <cstdio>
#include <cstdlib>

#include "Error.hpp"

void Error::errorAndExit(
    std::string _err_id,
    std::string _err_msg,
    std::string _err_arg) {
  /* TODO : call exit of child threads */
  printf("CPSimError (%s) : %s %s\n", _err_id.c_str(), _err_msg.c_str(), _err_arg.c_str());
  exit(-1);
}

