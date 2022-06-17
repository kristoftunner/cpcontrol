#ifndef ENERMAN_HPP
#define ENERMAN_HPP
#include <vector>
#include <map>

#include "json.hpp"
#include "device.hpp"

using json = nlohmann::json;

enum class EnermanReturnCode{
  ENERMAN_OK,
  ENERMAN_CONFIG_ERR
};

class Enerman {
private:
  DeviceContainer m_deviceContainer;
public:
  EnermanReturnCode BuildDevices(json devicesConfig);
  EnermanReturnCode Execute();
};

#endif //ENERMAN_HPP