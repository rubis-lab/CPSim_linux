/**
  *@File    CPSimMappingFunction.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   mapping function definitions for CPSimTask
  */
#include "CPSimMappingFunction.hpp"

CPSimTime_t Tau1_mapping_func(CPSimTime_t _e) {
  return _e / 10;
}

CPSimTime_t Tau2_mapping_func(CPSimTime_t _e) {
  return _e / 10;
}
