/**
  *@File    CPSimTypes.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   primitive type redefinition
  */
#ifndef __CPSimTypes__HPP__
#define __CPSimTypes__HPP__

typedef unsigned long long CPSimTime_t;

typedef void        (*CPSimFunction_t)        (void*);
typedef CPSimTime_t (*CPSimMappingFunction_t) (CPSimTime_t);

#endif
