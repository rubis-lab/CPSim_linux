/**
  *@File    CPSimTask.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimTask for KSWE RTSS 2017
  */
#ifndef __CPSimTask__HPP__
#define __CPSimTask__HPP__

#include <string>

#include "CPSimTypes.hpp"

class CPSimTask {
private:
  std::string             __name;     ///< identifier of the task (optional).
  CPSimFunction_t         __F;        ///< function of the task (page2-3).
  CPSimTime_t             __Phi;      ///< offset of the task (page2-3).
  CPSimTime_t             __P;        ///< period of the task (page2-3).
  CPSimTime_t             __C_best;   ///< best case execution time of the task (page2-3).
  CPSimTime_t             __C_worst;  ///< worst case execution time of the task (page2-3).
  CPSimMappingFunction_t  __M;        ///< exection time mapping function (page 3).

public:
  CPSimTask(void);
  CPSimTask(
      std::string             _name,
      CPSimFunction_t         _F,
      CPSimTime_t             _Phi,
      CPSimTime_t             _P,
      CPSimTime_t             _C_best,
      CPSimTime_t             _C_worst,
      CPSimMappingFunction_t  _M);

  void set_name     (std::string _name);
  void set_F        (CPSimFunction_t _F);
  void set_Phi      (CPSimTime_t _Phi);
  void set_P        (CPSimTime_t _P);
  void set_C_best   (CPSimTime_t _C_best);
  void set_C_worst  (CPSimTime_t _C_worst);
  void set_M        (CPSimMappingFunction_t _M);

  std::string             get_name    (void);
  CPSimFunction_t         get_F       (void);
  CPSimTime_t             get_Phi     (void);
  CPSimTime_t             get_P       (void);
  CPSimTime_t             get_C_best  (void);
  CPSimTime_t             get_C_worst (void);
  CPSimMappingFunction_t  get_M(void);
};

#endif
