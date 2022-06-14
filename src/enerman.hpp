#ifndef ENERMAN_HPP
#define ENERMAN_HPP
#include <vector>
#include <map>

#include "device.hpp"

enum class EnermanReturnCode{
  ENERMAN_OK,
  ENERMAN_CONFIG_ERR
};

class Enerman {
private:
  DeviceContainer m_deviceContainer;
public:
  EnermanReturnCode BuildDevices(std::vector<std::map<std::string,std::string>> deviceDict);
  EnermanReturnCode Execute();
};

#endif //ENERMAN_HPP