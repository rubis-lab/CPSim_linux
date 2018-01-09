/**
  *@File    CPSimCAN.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   CPSimCAN for KSWE RTSS 2017
  */
#include "CPSimCAN.hpp"

CPSimCAN::CPSimCAN(void) {

}

CPSimCAN::CPSimCAN(std::string _name) {
  this->__name = _name;
}

void CPSimCAN::set_name(std::string _name) {
  this->__name = _name;
}

std::string CPSimCAN::get_name(void) {
  return __name;
}

