/**
  *@File    CPSimSystem.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimSystem for KSWE RTSS 2017
  */
#include <cstdio>

#include "Error.hpp"
#include "CPSimSystem.hpp"

bool CPSimSystem::addCPSimTask (CPSimTask* _cpsim_task) {
  return __cpsim_tasks.insert(_cpsim_task).second;
}

bool CPSimSystem::addCPSimECU  (CPSimECU* _cpsim_ecu) {
  return __cpsim_ecus.insert(_cpsim_ecu).second;
}

bool CPSimSystem::addCPSimCAN  (CPSimCAN* _cpsim_can) {
  return __cpsim_cans.insert(_cpsim_can).second;
}

CPSimTask*  CPSimSystem::getCPSimTask  (std::string _cpsim_task_name) {
  
  CPSimTask to_find(_cpsim_task_name, NULL, 0, 0, 0, 0, NULL);
  
  auto it = __cpsim_tasks.find(&to_find);
  
  return (it == __cpsim_tasks.end()) ? NULL : *it;
}

CPSimECU*   CPSimSystem::getCPSimECU   (std::string _cpsim_ecu_name) {

  CPSimECU to_find(_cpsim_ecu_name, EDF);

  auto it = __cpsim_ecus.find(&to_find);

  return (it == __cpsim_ecus.end()) ? NULL : *it;
}

CPSimCAN*   CPSimSystem::getCPSimCAN   (std::string _cpsim_can_name) {
  
  CPSimCAN to_find(_cpsim_can_name);

  auto it = __cpsim_cans.find(&to_find);

  return (it == __cpsim_cans.end()) ? NULL : *it;
}

CPSimSystem::~CPSimSystem() {

  for (auto it = __cpsim_tasks.begin(); it != __cpsim_tasks.end(); ) {
    
    CPSimTask* to_del = *it;
    it = __cpsim_tasks.erase(it);
    
    delete to_del;
  }

  for (auto it = __cpsim_ecus.begin(); it != __cpsim_ecus.end(); ) {

    CPSimECU* to_del = *it;
    it = __cpsim_ecus.erase(it);

    delete to_del;
  }

  for (auto it = __cpsim_cans.begin(); it != __cpsim_cans.end(); ) {

    CPSimCAN* to_del = *it;
    it = __cpsim_cans.erase(it);

    delete to_del;
  }
}

bool CPSimSystem::readDesign(std::string _fname) {
  FILE* fp = fopen(_fname.c_str(), "r");
  if (fp == NULL)
  	Error::errorAndExit("@CPSimSystem::readDesign", "File open failed", _fname);



  fclose(fp);
}
