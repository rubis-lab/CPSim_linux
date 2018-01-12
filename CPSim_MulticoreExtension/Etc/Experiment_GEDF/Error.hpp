/**
  *@File    Error.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   All of the error messages are printed here
  */
#ifndef __Error__HPP__
#define __Error__HPP__

#include <string>

class Error {
public:
  static void errorAndExit(
      std::string _err_id,
      std::string _err_msg,
      std::string _err_arg);
};

#endif
