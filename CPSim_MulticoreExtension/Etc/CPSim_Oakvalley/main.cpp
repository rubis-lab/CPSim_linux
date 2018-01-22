/**
  *@File    main.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   main for GEDF experimental code
  */
#include <cstdio>

#include "CPSimSystem.hpp"
#include "CPSimTask.hpp"
#include "CPSimFunction.hpp"
#include "CPSimMappingFunction.hpp"
#include "Logger.hpp"

int main(int argc, char* argv[])
{
  CPSimSystem sys;

  /* Add tasks */
  sys.addCPSimTask(new CPSimTask("Tau1", Tau1_func, 0, 1000000000, 1000000000, 1000000000, Tau1_mapping_func));
  sys.addCPSimTask(new CPSimTask("Tau2", Tau2_func, 0, 100000000, 100000000, 100000000, Tau2_mapping_func));

  /* Run system */

  /* Print summary */
  Logger::printLog();

  return 0;
}
