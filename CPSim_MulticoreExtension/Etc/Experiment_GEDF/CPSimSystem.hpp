/**
  *@File    CPSimSystem.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimSystem for KSWE RTSS 2017
  */
#ifndef __CPSimSystem__HPP__
#define __CPSimSystem__HPP__

#include <set>
#include <string>

#include "CPSimTask.hpp"
#include "CPSimECU.hpp"
#include "CPSimCAN.hpp"

template <typename T>
struct CompareComponentPtrLess {
  bool operator() (const T* _lhs, const T* _rhs) const {
    return (_lhs->get_name().compare(_rhs->get_name()) < 0);
  }
};

template <typename T>
struct CompareComponentPtrGreater {
  bool operator() (const T* _lhs, const T* _rhs) const {
    return !(_lhs->get_name().compare(_rhs->get_name()) < 0);
  }
};

class CPSimSystem {
private:
  std::set<CPSimTask*,  CompareComponentPtrLess<CPSimTask> > __cpsim_tasks;
  std::set<CPSimECU*,   CompareComponentPtrLess<CPSimECU> >  __cpsim_ecus;
  std::set<CPSimCAN*,   CompareComponentPtrLess<CPSimCAN> >  __cpsim_cans;

  bool addCPSimTask (CPSimTask* _cpsim_task);
  bool addCPSimECU  (CPSimECU* _cpsim_ecu);
  bool addCPSimCAN  (CPSimCAN* _cpsim_can);

  CPSimTask*  getCPSimTask  (std::string _cpsim_task_name);
  CPSimECU*   getCPSimECU   (std::string _cpsim_ecu_name);
  CPSimCAN*   getCPSimCAN   (std::string _cpsim_can_name);

public:
  ~CPSimSystem();

  bool readDesign(std::string _fname);
};

#endif
