/**
  *@File    CPSimECU.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimECU for KSWE RTSS 2017
  */
#include "CPSimECU.hpp"

CPSimECU::CPSimECU(void) {

}

CPSimECU::CPSimECU(
    std::string       _name,
    ECU_SCHED_POLICY  _sched_policy) {
  this->__name          = _name;
  this->__sched_policy  = _sched_policy;
}

void CPSimECU::set_name         (std::string _name) {
  this->__name = _name;
}

void CPSimECU::set_sched_policy (ECU_SCHED_POLICY _sched_policy) {
  this->__sched_policy = _sched_policy;
}

std::string       CPSimECU::get_name          (void) const {
  return __name;
}

ECU_SCHED_POLICY  CPSimECU::get_sched_policy  (void) const {
  return __sched_policy;
}
