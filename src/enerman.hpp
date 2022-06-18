#ifndef ENERMAN_HPP
#define ENERMAN_HPP
#include <vector>
#include <map>

#include "json.hpp"
#include "device.hpp"

using json = nlohmann::json;

enum class EnermanReturnCode{
  ENERMAN_OK,
  ENERMAN_CONFIG_ERR,
  ENERMAN_READ_ERR
};

class Enerman {
private:
  DeviceContainer m_deviceContainer;
  std::map<std::string, std::shared_ptr<ModbusPort>> m_modbusCommPortMap;
public:
  EnermanReturnCode BuildDevices(json devicesConfig);
  EnermanReturnCode Execute();
};

#endif //ENERMAN_HPP