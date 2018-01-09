/**
  *@File    CPSimECU.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimECU for KSWE RTSS 2017
  */
#ifndef __CPSimECU__HPP__
#define __CPSimECU__HPP__

#include <string>

typedef enum _ECU_SCHED_POLICY {
  RM,
  EDF
} ECU_SCHED_POLICY;

class CPSimECU {
private:
  std::string       __name;         ///< name of the ECU (optional).
  ECU_SCHED_POLICY  __sched_policy; ///< scheduling policy of the ECU (page 2).

public:
  CPSimECU(void);
  CPSimECU(
      std::string       _name,
      ECU_SCHED_POLICY  _sched_policy);

  void set_name         (std::string _name);
  void set_sched_policy (ECU_SCHED_POLICY _sched_policy);

  std::string       get_name          (void);
  ECU_SCHED_POLICY  get_sched_policy  (void);
};

#endif
