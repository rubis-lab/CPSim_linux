/**
  *@File    CPSimTask.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimTask for KSWE RTSS 2017
  */
#include "CPSimTask.hpp"

CPSimTask::CPSimTask(void) {

}

CPSimTask::CPSimTask(
    std::string             _name,
    CPSimFunction_t         _F,
    CPSimTime_t             _Phi,
    CPSimTime_t             _P,
    CPSimTime_t             _C_best,
    CPSimTime_t             _C_worst,
    CPSimMappingFunction_t  _M) {
  this->__name    = _name;
  this->__F       = _F;
  this->__Phi     = _Phi;
  this->__P       = _P;
  this->__C_best  = _C_best;
  this->__C_worst = _C_worst;
  this->__M       = _M;
}

void CPSimTask::set_name     (std::string _name) {
  this->__name = _name;
}

void CPSimTask::set_F        (CPSimFunction_t _F) {
  this->__F = _F;
}

void CPSimTask::set_Phi      (CPSimTime_t _Phi) {
  this->__Phi = _Phi;
}

void CPSimTask::set_P        (CPSimTime_t _P) {
  this->__P = _P;
}

void CPSimTask::set_C_best   (CPSimTime_t _C_best) {
  this->__C_best = _C_best;
}

void CPSimTask::set_C_worst  (CPSimTime_t _C_worst) {
  this->__C_worst = _C_worst;
}

void CPSimTask::set_M        (CPSimMappingFunction_t _M) {
  this->__M = _M;
}

std::string             CPSimTask::get_name    (void) const {
  return __name;
}

CPSimFunction_t         CPSimTask::get_F       (void) const {
  return __F;
}

CPSimTime_t             CPSimTask::get_Phi     (void) const {
  return __Phi;
}

CPSimTime_t             CPSimTask::get_P       (void) const {
  return __P;
}

CPSimTime_t             CPSimTask::get_C_best  (void) const {
  return __C_best;
}

CPSimTime_t             CPSimTask::get_C_worst (void) const {
  return __C_worst;
}

CPSimMappingFunction_t  CPSimTask::get_M(void) const {
  return __M;
}
