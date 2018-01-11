/**
  *@File    CPSimCAN.hpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimCAN for KSWE RTSS 2017
  */
#ifndef __CPSimCAN__HPP__
#define __CPSimCAN__HPP__

#include <string>

class CPSimCAN {
private:
  std::string __name; ///< name of the CAN (optional).

public:
  CPSimCAN(void);
  CPSimCAN(std::string _name);

  void set_name(std::string _name);

  std::string get_name(void) const;
};

#endif
