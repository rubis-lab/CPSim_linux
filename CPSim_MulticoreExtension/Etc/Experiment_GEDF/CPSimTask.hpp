/**
  *@File    CPSimTask.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimTask for KSWE RTSS 2017
  */
#ifndef __CPSimTask__HPP__
#define __CPSimTask__HPP__

#include <string>

class CPSimTask {
private:
  std::string name;
  void F(void* arg);

public:
  CPSimTask(void);
  ~CPSimTask(void);
};

#endif
